CREATE TABLE IF NOT EXISTS users (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    google_id     TEXT    UNIQUE NOT NULL,
    email         TEXT    UNIQUE NOT NULL,
    name          TEXT    NOT NULL,
    picture       TEXT,
    bio           TEXT    DEFAULT '',
    age           INTEGER CHECK(age IS NULL OR (age >= 18 AND age <= 120)),
    gender        TEXT    CHECK(gender IN ('male', 'female', 'non_binary', 'other')),
    interested_in TEXT    CHECK(interested_in IN ('male', 'female', 'everyone')),
    latitude      REAL,
    longitude     REAL,
    max_distance  INTEGER DEFAULT 50,
    created_at    INTEGER NOT NULL DEFAULT (unixepoch()),
    updated_at    INTEGER NOT NULL DEFAULT (unixepoch())
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
