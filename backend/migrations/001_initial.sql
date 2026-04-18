CREATE TABLE IF NOT EXISTS users (
    id                 INTEGER PRIMARY KEY AUTOINCREMENT,
    name               TEXT    NOT NULL UNIQUE,
    email              TEXT    UNIQUE,

    -- Collected from user
    age                INTEGER CHECK(age IS NULL OR (age >= 18 AND age <= 120)),
    location           TEXT,
    phone_number       TEXT,

    -- Projected by enrichment pipeline
    projected_gender   TEXT,
    projected_ethnicity TEXT,
    education_level    TEXT,
    political_belief   REAL,

    -- Enrichment lifecycle
    enrichment_status  TEXT    NOT NULL DEFAULT 'pending',
    enriched_at        INTEGER,

    picture            TEXT,
    created_at         INTEGER NOT NULL DEFAULT (unixepoch()),
    updated_at         INTEGER NOT NULL DEFAULT (unixepoch())
);

CREATE TABLE IF NOT EXISTS user_photos (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id       INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    url           TEXT    NOT NULL,
    display_order INTEGER NOT NULL DEFAULT 0,
    created_at    INTEGER NOT NULL DEFAULT (unixepoch())
);

CREATE TABLE IF NOT EXISTS likes (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    liker_id   INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    liked_id   INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    created_at INTEGER NOT NULL DEFAULT (unixepoch()),
    UNIQUE(liker_id, liked_id)
);

CREATE TABLE IF NOT EXISTS matches (
    id         INTEGER PRIMARY KEY AUTOINCREMENT,
    user1_id   INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    user2_id   INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    created_at INTEGER NOT NULL DEFAULT (unixepoch()),
    UNIQUE(user1_id, user2_id),
    CHECK(user1_id < user2_id)
);

CREATE INDEX IF NOT EXISTS idx_likes_liker   ON likes(liker_id);
CREATE INDEX IF NOT EXISTS idx_likes_liked   ON likes(liked_id);
CREATE INDEX IF NOT EXISTS idx_matches_user1 ON matches(user1_id);
CREATE INDEX IF NOT EXISTS idx_matches_user2 ON matches(user2_id);
CREATE INDEX IF NOT EXISTS idx_photos_user   ON user_photos(user_id);
