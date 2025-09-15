#include "Renderer/Character.h"

Character::Character() : Model() {}

Character::Character(const std::filesystem::path& filePath) : Model(filePath) {}

void Character::Update(float deltaTime)
{
    // character의 이동방향에 따른 방향 정하기
    if (m_moveLeftRight > 0) {
        if (m_moveBackForward == 0)
            m_targetRotation = CharacterDirection::RIGHT;
        else if (m_moveBackForward > 0) 
            m_targetRotation = CharacterDirection::UP_RIGHT;
        else 
            m_targetRotation = CharacterDirection::DOWN_RIGHT;
    } 
    else if (m_moveLeftRight == 0) {
        if (m_moveBackForward == 0) ;
        else if (m_moveBackForward > 0)
            m_targetRotation = CharacterDirection::UP;
        else
            m_targetRotation = CharacterDirection::DOWN;
    }
    else {
        if (m_moveBackForward == 0)
            m_targetRotation = CharacterDirection::LEFT;
        else if (m_moveBackForward > 0)
            m_targetRotation = CharacterDirection::UP_LEFT;
        else
            m_targetRotation = CharacterDirection::DOWN_LEFT;
    }


    // 방향이 바뀌었으면 회전 처리
    if (m_targetRotation != m_currentRotation) {
        // 각도 계산 (UP=0, UP_LEFT=1, LEFT=2, ... )
        float angle = (static_cast<int>(m_currentRotation) - static_cast<int>(m_targetRotation)) * 45.0f;  // degree
        RotateYInObjectCoordinate(XMConvertToRadians(angle), m_currentPosition);  // radian 사용하여 회전

        m_currentRotation = m_targetRotation;
    }

    // input handler에 따라 이동할 위치 계산
    m_targetPosition += m_moveLeftRight * DEFAULT_RIGHT;
    m_targetPosition += m_moveBackForward * DEFAULT_FORWARD;
    // 움직일 vector
    XMVECTOR moveVector = m_targetPosition - m_currentPosition;

    // 이동 적용
    Translate(moveVector);

    // 현재 위치 갱신
    m_currentPosition = m_targetPosition;

    // 이동량 변수 초기화
    m_moveLeftRight = 0.0f;
    m_moveBackForward = 0.0f;
}

void Character::RotateYInObjectCoordinate(float angle, XMVECTOR objectOffset)
{
    // world origin으로 이동
    Translate(-objectOffset);
    // Y축 회전
    RotateY(angle);
    // 다시 원래 위치로 복귀
    Translate(objectOffset);
}

void Character::HandleInput(const InputDirections& directions, float deltaTime)
{
    // input에 따라 이동량  결정
    if (directions.bFront) {
        m_moveBackForward += m_movementSpeed * deltaTime;
    }
    if (directions.bBack) {
        m_moveBackForward -= m_movementSpeed * deltaTime;
    }
    if (directions.bLeft) {
        m_moveLeftRight -= m_movementSpeed * deltaTime;
    }
    if (directions.bRight) {
        m_moveLeftRight += m_movementSpeed * deltaTime;
    }
}
