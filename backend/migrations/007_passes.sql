-- Records profiles a user explicitly skipped in Discover so they don't reappear.
CREATE TABLE IF NOT EXISTS passes (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    passer_id  INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    passed_id  INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    created_at INTEGER NOT NULL DEFAULT (unixepoch()),
    UNIQUE(passer_id, passed_id)
);

CREATE INDEX IF NOT EXISTS idx_passes_passer ON passes(passer_id);
