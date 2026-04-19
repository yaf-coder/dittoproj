CREATE TABLE IF NOT EXISTS users (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,

    -- Credentials
    phone_number   TEXT    UNIQUE NOT NULL,
    password_hash  TEXT    NOT NULL,

    -- Collected from user
    name           TEXT    NOT NULL,
    age            INTEGER CHECK(age IS NULL OR (age >= 18 AND age <= 120)),
    location       TEXT,

    -- Optional enrichment fields (user-supplied via profile form)
    gender           TEXT,   -- "male" | "female" | "non_binary" | "other" | "prefer_not_to_say"
    ethnicity        TEXT,   -- "white" | "black" | "hispanic" | "asian" | "native_american" | "pacific_islander" | "middle_eastern" | "other" | "prefer_not_to_say"
    education_level  TEXT,   -- "high_school" | "some_college" | "bachelors" | "masters" | "phd"
    study_location   TEXT,   -- free text, e.g. "MIT" or "University of Texas"
    political_belief REAL,   -- 0.0 = very liberal, 1.0 = very conservative

    -- Profile photo (user-uploaded)
    picture        TEXT,

    created_at     INTEGER NOT NULL DEFAULT (unixepoch()),
    updated_at     INTEGER NOT NULL DEFAULT (unixepoch())
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
