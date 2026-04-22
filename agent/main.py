import json
import os
import sqlite3
from fastapi import FastAPI
from pydantic import BaseModel
import google.generativeai as genai

app = FastAPI()

GEMINI_API_KEY = "AIzaSyBkrzJwZb10dx6N_xGkWFfK4i3qoACTQ9A"
DB_PATH = os.getenv("DB_PATH", "../backend/ditto.db")
TOP_N = 3

genai.configure(api_key=GEMINI_API_KEY)
model = genai.GenerativeModel("gemini-2.0-flash")

QUESTIONS = {
    1:  "What does your ideal free weekend usually look like?",
    2:  "When you're stressed or overwhelmed, what do you usually do?",
    3:  "What does a really good relationship look like to you day-to-day?",
    4:  "Tell me about a time you disagreed with someone close to you — how'd that go?",
    5:  "What are you most focused on in your life right now?",
    6:  "How important is ambition or career stuff in a partner to you?",
    7:  "What are some things you just won't compromise on in a relationship?",
    8:  "How do you usually show and like to receive affection?",
    9:  "What's your ideal social life like?",
    10: "What's something about you people don't realize right away?",
}

EDUCATION_LABELS = {
    "high_school": "High School", "some_college": "Some College",
    "bachelors": "Bachelor's", "masters": "Master's", "phd": "PhD",
}

POLITICAL_LABELS = [
    (0.125, "Very Liberal"), (0.375, "Liberal"), (0.625, "Moderate"),
    (0.875, "Conservative"), (1.001, "Very Conservative"),
]


def get_political_label(score):
    for threshold, label in POLITICAL_LABELS:
        if score < threshold:
            return label
    return "Very Conservative"


def get_user(conn, user_id):
    row = conn.execute("""
        SELECT id, name, age, gender, sexual_orientation,
               education_level, study_location, political_belief, location
        FROM users WHERE id = ?
    """, (user_id,)).fetchone()
    if not row:
        return None, []

    responses = conn.execute("""
        SELECT question_id, answer FROM user_responses WHERE user_id = ?
        ORDER BY created_at DESC
    """, (user_id,)).fetchall()

    return row, responses


def format_profile(row, responses):
    id_, name, age, gender, orientation, education, study_loc, political, location = row
    lines = [f"Name: {name or 'Unknown'}"]
    if age:         lines.append(f"Age: {age}")
    if gender:      lines.append(f"Gender: {gender.replace('_', ' ')}")
    if orientation: lines.append(f"Sexual orientation: {orientation.replace('_', ' ')}")
    if education:   lines.append(f"Education: {EDUCATION_LABELS.get(education, education)}")
    if study_loc:   lines.append(f"Studied at: {study_loc}")
    if location:    lines.append(f"Location: {location}")
    if political is not None:
        lines.append(f"Political identity: {get_political_label(float(political))}")

    if responses:
        lines.append("\nPersonality responses:")
        for qid, answer in responses:
            question = QUESTIONS.get(qid, f"Question {qid}")
            lines.append(f"Q: {question}\nA: {answer}")

    return "\n".join(lines)


def compute_score(user1_row, user1_responses, user2_row, user2_responses):
    prompt = f"""You are a compatibility scoring engine for a dating app. Given two user profiles and their responses to personality questions, output a compatibility score between 0.0 and 1.0.

Consider: shared values, complementary personalities, lifestyle fit, communication styles, relationship goals, and any dealbreakers mentioned.

User 1:
{format_profile(user1_row, user1_responses)}

User 2:
{format_profile(user2_row, user2_responses)}

Respond with ONLY a JSON object, no markdown fences:
{{"score": 0.00, "reason": "one concise sentence"}}"""

    response = model.generate_content(prompt)
    text = response.text.strip()

    # Strip markdown fences if the model wraps them anyway
    if text.startswith("```"):
        lines = text.splitlines()
        text = "\n".join(lines[1:-1] if lines[-1] == "```" else lines[1:])

    result = json.loads(text.strip())
    score = max(0.0, min(1.0, float(result["score"])))
    reason = result.get("reason", "")
    return score, reason


class ComputeRequest(BaseModel):
    user_id: int


@app.post("/compute")
def compute(req: ComputeRequest):
    conn = sqlite3.connect(DB_PATH)
    try:
        user1_row, user1_responses = get_user(conn, req.user_id)
        if user1_row is None:
            return {"error": "User not found"}

        other_ids = [r[0] for r in conn.execute(
            "SELECT id FROM users WHERE id != ?", (req.user_id,)
        ).fetchall()]

        if not other_ids:
            return {"success": True, "scores_computed": 0}

        scores = []
        for other_id in other_ids:
            user2_row, user2_responses = get_user(conn, other_id)
            if user2_row is None:
                continue
            try:
                score, reason = compute_score(
                    user1_row, user1_responses,
                    user2_row, user2_responses,
                )
                scores.append((other_id, score, reason))
            except Exception as e:
                print(f"Error scoring ({req.user_id}, {other_id}): {e}")

        scores.sort(key=lambda x: x[1], reverse=True)
        top = scores[:TOP_N]

        conn.execute("DELETE FROM compatibility_scores WHERE user_id = ?", (req.user_id,))
        for match_id, score, reason in top:
            conn.execute(
                "INSERT INTO compatibility_scores (user_id, match_id, score, reason) VALUES (?, ?, ?, ?)",
                (req.user_id, match_id, score, reason),
            )
        conn.commit()

        return {"success": True, "scores_computed": len(top)}
    finally:
        conn.close()


@app.get("/scores/{user_id}")
def get_scores(user_id: int):
    """Returns stored compatibility scores for a user, highest first."""
    conn = sqlite3.connect(DB_PATH)
    try:
        rows = conn.execute("""
            SELECT cs.match_id, u.name, cs.score, cs.reason, cs.computed_at
            FROM compatibility_scores cs
            JOIN users u ON u.id = cs.match_id
            WHERE cs.user_id = ?
            ORDER BY cs.score DESC
        """, (user_id,)).fetchall()
        return [
            {"match_id": r[0], "name": r[1], "score": r[2],
             "reason": r[3], "computed_at": r[4]}
            for r in rows
        ]
    finally:
        conn.close()
