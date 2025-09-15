# Computer-Graphics
KOREA University, COSE331 Computer Graphics class

# 프로젝트 개요

- 3차원 세계를 2차원 화면으로 투영해주는 GPU Pipeline에 대해 공부하고 직접 구현해보는 프로젝트입니다.
- GPU Pipeline 중 Input Assembler, Vertex Shader, Pixel Shader, Output Merger 단계를 작성해보며 렌더링 과정을 이해해보았습니다.
  <img width="1916" height="250" alt="image" src="https://github.com/user-attachments/assets/876c4549-d439-4694-b231-75d4b817cf5e" />


# GPU Pipeline basic

- GPU Pipeline을 기초적으로 구현한 프로젝트입니다.
  
https://github.com/user-attachments/assets/0080a9ef-157b-4163-af3d-2acf86b258a0

---

- Build: 프로젝트를 실행할 수 있는 솔루션 파일(Build.sln) 존재
- Data: 렌더링할 정육면체 obj파일 존재
- External: 텍스쳐를 입히기 위한 assimp 폴더 존재
- Source: GPU Pipeline 구현을 위한 cpp, h파일 존재

---

- InitWindow: window 창을 생성하고 화면에 뛰어주는 함수
  <img width="1201" height="532" alt="image" src="https://github.com/user-attachments/assets/969d5783-a71b-4842-9061-bee7c8e678d4" />
  <img width="829" height="114" alt="image" src="https://github.com/user-attachments/assets/e1842fb0-a475-47e7-ae0e-f31dedb51d5c" />


- InitDevice: GPU Pipeline을 위한 Resource 생성
  - Render Target, Back Buffer, Depth Buffer, ViewPort
  - Vertex Buffer, Index Buffer
  - Vertex Shader, Pixel Shader, Input Layout 등
 
 - Render: InitDevice으로 생성한 resource를 GPU에 binding하는 과정. Draw호출하여 gpu pipeline 실행
   - 순서 :	color/depth buffer 초기화 (ClearRenderTargetView, ClearDepthStencilView)
   - 업데이트 필요한 constant buffer(광원, WVM matrix, texture...)들 update
   - Pipeline state(셰이더, 버퍼 등) 설정 및 binding
   - Draw로 오브젝트 렌더링
   - 다시 반복하며 object 렌더링 후, 마지막에 present로 front/back buffer swap

  <br>
  
  - 시간 Frame 당 회전 Rotation Matrix 계산
      <img width="1266" height="514" alt="image" src="https://github.com/user-attachments/assets/670504f5-8277-4034-a0f4-b4acea230e72" />



# Golem Rendering

- GPU Pipeline을 이용한 위 프로젝트를 기반으로 골렘을 렌더링하고, 방향키에 따른 이동모션까지 넣어본 프로젝트입니다.

- 위 프로젝트에서 단일책임원칙을 준수하며 새로운 객체들로 기능을 나누어 구현하였습니다.
  
https://github.com/user-attachments/assets/deb34f53-4cab-4818-9e95-2d69fa33082f

---

- Build: 프로젝트를 실행할 수 있는 솔루션 파일(Build.sln) 존재
- Data: 렌더링할 골렘에 대한 텍스쳐 파일 및 obj 파일
- External: 텍스쳐를 입히기 위한 assimp 폴더 존재
- Source: GPU Pipeline 구현을 위한 cpp, h파일 존재

---

- main: 렌더링 할 광원 큐브 및 골렘을 Renderer에 등록
- Renderable: 렌더링 되어질 객체의 가장 기본, 부모 클래스.
- BaseCube: Cube 형태의 객체의 가장 기본이 되는 부모 클래스. 회전하는 Rotation Cube 객체는 이 클래스를 상속받음.
- Model: 텍스쳐 파일을 가지는 객체 클래스. Renderable 클래스를 상속받아 mesh, material 등을 사용함.
- Character: 키보드 입력을 받아 이동 및 회전 기능을 가지는 객체 클래스. Model 클래스를 상속받아 골렘 캐릭터를 생성.
- Game: 전체적인 게임 구조로, loop 에서 입력을 처리하거나 입력이 없을 경우 Render 실행하여 렌더링.
  
- Renderer: 모든 렌더링 역할을 담당하는 객체.

  
- Shader.fx: Vertex Shader, Pixel Shader 구현
  
  <img width="800" height="254" alt="image" src="https://github.com/user-attachments/assets/d78aac2a-6abc-4595-ad4a-60e0a8b320a3" />
  <img width="800" height="344" alt="image" src="https://github.com/user-attachments/assets/158bb10c-e6df-4878-9df8-823c7bccd912" />


- 플레이어의 입력을 받아 골렘의 vertex들을 움직여주었고 그에 따라 카메라의 시점도 함께 바꾸어주었습니다.
  
<img width="387" height="299" alt="image" src="https://github.com/user-attachments/assets/07f31be1-59ff-4940-838f-595b2c3a0089" />
<img width="362" height="244" alt="image" src="https://github.com/user-attachments/assets/db61176e-d7e4-4206-b50a-2cf8da3a6d94" />


  

- Assimp, Phong Lighting을 이용하여 골렘의 외형을 표현했습니다.

