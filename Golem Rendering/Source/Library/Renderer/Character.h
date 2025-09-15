#pragma once
#include "Renderer/Model.h"

enum class CharacterDirection
{
    UP,
    UP_LEFT,
    LEFT,
    DOWN_LEFT,
    DOWN,
    DOWN_RIGHT,
    RIGHT,
    UP_RIGHT
};

class Character : public Model
{
public:
    Character();
    Character(const std::filesystem::path& filePath);
    virtual ~Character() = default;

    void HandleInput(const InputDirections& directions, float deltaTime);
    void Update(float deltaTime) override;

    // ���� �䱸: ������Ʈ�� ���� ��ǥ�� ���� Y�� ȸ��
    void RotateYInObjectCoordinate(float angle, XMVECTOR objectOffset);

private:
    static constexpr const XMVECTORF32 DEFAULT_FORWARD = { 0.0f, 0.0f, 1.0f, 0.0f };
    static constexpr const XMVECTORF32 DEFAULT_RIGHT = { 1.0f, 0.0f, 0.0f, 0.0f };

    XMVECTOR m_targetPosition = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    XMVECTOR m_currentPosition = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
    float m_moveLeftRight = 0.0f;
    float m_moveBackForward = 0.0f;
    CharacterDirection m_currentRotation = CharacterDirection::DOWN;
    CharacterDirection m_targetRotation = CharacterDirection::DOWN;
    float m_movementSpeed = 10.0f;
};
