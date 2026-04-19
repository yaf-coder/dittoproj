CREATE TABLE IF NOT EXISTS user_responses (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id     INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    question_id INTEGER NOT NULL,
    answer      TEXT    NOT NULL,
    created_at  INTEGER NOT NULL DEFAULT (unixepoch())
);

CREATE INDEX IF NOT EXISTS idx_responses_user ON user_responses(user_id);
