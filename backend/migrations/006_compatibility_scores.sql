-- Stores the top-3 compatibility scores per user, computed by the Python agent.
-- user_id is the user we computed FOR; match_id is the candidate.
CREATE TABLE IF NOT EXISTS compatibility_scores (
    user_id     INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    match_id    INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    score       REAL    NOT NULL CHECK(score >= 0.0 AND score <= 1.0),
    reason      TEXT,
    computed_at INTEGER NOT NULL DEFAULT (unixepoch()),
    PRIMARY KEY (user_id, match_id)
);

CREATE INDEX IF NOT EXISTS idx_scores_user_score ON compatibility_scores(user_id, score DESC);
