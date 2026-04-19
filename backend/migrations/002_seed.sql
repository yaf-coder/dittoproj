-- Seed user: phone=959-929-2482, password=yafee (SHA-256)
INSERT INTO users (name, phone_number, password_hash)
VALUES ('yafee', '959-929-2482', '942478d27dd571b09210032bf29fe7c2d4776aa513372612ae2796249a169ad8')
ON CONFLICT(phone_number) DO NOTHING;
