\c furbbs;

-- ==================== 实名验证相关表 ====================

CREATE TABLE IF NOT EXISTS real_name_verifications (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    name VARCHAR(128) NOT NULL,
    id_card_number VARCHAR(64) NOT NULL,
    is_verified BOOLEAN DEFAULT FALSE,
    is_matched BOOLEAN DEFAULT FALSE,
    confidence_level INTEGER DEFAULT 0,
    task_id VARCHAR(128),
    transaction_id VARCHAR(128),
    status VARCHAR(64) DEFAULT 'pending',
    reason VARCHAR(512),
    face_verified BOOLEAN DEFAULT FALSE,
    face_similarity FLOAT DEFAULT 0.0,
    verify_provider VARCHAR(64) DEFAULT 'netease',
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000,
    verified_at BIGINT,
    retry_count INTEGER DEFAULT 0
);

CREATE INDEX IF NOT EXISTS idx_verification_user ON real_name_verifications(user_id);
CREATE INDEX IF NOT EXISTS idx_verification_task ON real_name_verifications(task_id);
CREATE INDEX IF NOT EXISTS idx_verification_status ON real_name_verifications(status);

CREATE TABLE IF NOT EXISTS face_verification_records (
    id SERIAL PRIMARY KEY,
    user_id VARCHAR(128) NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    task_id VARCHAR(128),
    image_url VARCHAR(512),
    similarity FLOAT DEFAULT 0.0,
    is_passed BOOLEAN DEFAULT FALSE,
    best_face_url VARCHAR(512),
    created_at BIGINT DEFAULT EXTRACT(EPOCH FROM NOW()) * 1000
);

CREATE INDEX IF NOT EXISTS idx_face_verify_user ON face_verification_records(user_id);
CREATE INDEX IF NOT EXISTS idx_face_verify_task ON face_verification_records(task_id);
