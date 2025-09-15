#include "Renderer/Character.h"

Character::Character() : Model() {}

Character::Character(const std::filesystem::path& filePath) : Model(filePath) {}

void Character::Update(float deltaTime)
{
    // character�� �̵����⿡ ���� ���� ���ϱ�
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


    // ������ �ٲ������ ȸ�� ó��
    if (m_targetRotation != m_currentRotation) {
        // ���� ��� (UP=0, UP_LEFT=1, LEFT=2, ... )
        float angle = (static_cast<int>(m_currentRotation) - static_cast<int>(m_targetRotation)) * 45.0f;  // degree
        RotateYInObjectCoordinate(XMConvertToRadians(angle), m_currentPosition);  // radian ����Ͽ� ȸ��

        m_currentRotation = m_targetRotation;
    }

    // input handler�� ���� �̵��� ��ġ ���
    m_targetPosition += m_moveLeftRight * DEFAULT_RIGHT;
    m_targetPosition += m_moveBackForward * DEFAULT_FORWARD;
    // ������ vector
    XMVECTOR moveVector = m_targetPosition - m_currentPosition;

    // �̵� ����
    Translate(moveVector);

    // ���� ��ġ ����
    m_currentPosition = m_targetPosition;

    // �̵��� ���� �ʱ�ȭ
    m_moveLeftRight = 0.0f;
    m_moveBackForward = 0.0f;
}

void Character::RotateYInObjectCoordinate(float angle, XMVECTOR objectOffset)
{
    // world origin���� �̵�
    Translate(-objectOffset);
    // Y�� ȸ��
    RotateY(angle);
    // �ٽ� ���� ��ġ�� ����
    Translate(objectOffset);
}

void Character::HandleInput(const InputDirections& directions, float deltaTime)
{
    // input�� ���� �̵���  ����
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
