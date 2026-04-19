INSERT INTO users (name, phone_number) VALUES ('yafee', '959-929-2482')
ON CONFLICT(phone_number) DO NOTHING;
