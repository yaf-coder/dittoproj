-- Migration: replace inferred projection columns with user-supplied optional fields.
-- Requires SQLite 3.25+ (RENAME COLUMN) and 3.35+ (DROP COLUMN).
-- Safe to run on fresh DBs only if 001_initial.sql has already run.

ALTER TABLE users RENAME COLUMN projected_gender    TO gender;
ALTER TABLE users RENAME COLUMN projected_ethnicity TO ethnicity;
ALTER TABLE users DROP COLUMN enrichment_status;
ALTER TABLE users DROP COLUMN enriched_at;
