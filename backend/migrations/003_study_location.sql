-- Migration: add study_location column for existing databases.
ALTER TABLE users ADD COLUMN study_location TEXT;
