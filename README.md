# 프로젝트 개요

- 3차원 세계를 2차원 화면으로 투영해주는 GPU Pipeline에 대해 공부하고 visual studio를 이용하여 직접 구현해보는 프로젝트입니다.

 - 골렘 캐릭터를 렌더링하고, 방향키 입력에 따라 이동하게 만들었습니다.
 
 - Phong Lighting Model을 구현하여 광원을 만들었습니다.

   https://github.com/user-attachments/assets/deb34f53-4cab-4818-9e95-2d69fa33082f


# 디렉토리 구조

- Build: 프로젝트를 실행할 수 있는 솔루션 파일(Build.sln) 존재
- Data: 렌더링할 골렘에 대한 텍스쳐 파일 및 obj 파일
- External: 텍스쳐를 입히기 위한 assimp 폴더 존재
- Source: GPU Pipeline 구현을 위한 cpp, h파일 존재


# 프로젝트 세부 설명

- GPU Pipeline 중 Input Assembler, Vertex Shader, Pixel Shader, Output Merger 단계를 작성해보며 렌더링 과정을 이해해보았습니다.
  <img width="1916" height="250" alt="image" src="https://github.com/user-attachments/assets/876c4549-d439-4694-b231-75d4b817cf5e" >

- GPU Resources (SwapChain, backbuffer, DepthStencilView, Viewport, Shader 등)를 생성하고 binding하는 과정에 대해 공부하였습니다.

- 객체지향적 설계를 유지하며, SOLID 원칙을 준수하도록 구현하였습니다.
  (전체적인 UML Diagram)
  <img width="923" height="580" alt="image" src="https://github.com/user-attachments/assets/fe02fb26-bcae-49b0-8738-304f4c40a18f" />

# 객체 구현

- Game
   <img width="646" height="327" alt="image" src="https://github.com/user-attachments/assets/c7dfd309-0153-4818-9281-25ae0400185a" />
  - Main Window와 Render 객체를 통해 게임을 진행합니다.
  - Window에서 받은 입력을 처리하거나 입력이 없을 경우 Render 객체를 이용하여 렌더링합니다.
 
  <img width="800" height="450" alt="image" src="https://github.com/user-attachments/assets/29a74e4c-0e6a-490c-b13e-1f837ab6977a" />

- Renderer
  <img width="1347" height="464" alt="image" src="https://github.com/user-attachments/assets/f7ae8639-1265-44d9-b144-22b516efe4f2" />
   - 렌더링을 담당하는 객체로, 렌더링 후 화면에 표시까지 해줍니다.
   - 순서 :	color/depth buffer 초기화 (ClearRenderTargetView, ClearDepthStencilView)
   - 업데이트 필요한 constant buffer(광원, WVM matrix, texture...)들 update
   - Pipeline state(셰이더, 버퍼 등) 설정 및 binding
   - Draw로 오브젝트 렌더링
   - 다시 반복하며 object 렌더링 후, 마지막에 present로 front/back buffer swap


- 재사용 가능한 객체 Renderable, BaseCube, Model, Character를 구현하여 렌더링하였습니다.
  <img width="661" height="444" alt="image" src="https://github.com/user-attachments/assets/fd65f5d7-37f8-441c-95f7-72add46b4762" />

  - Renderable: 렌더링 되어질 객체의 가장 기본, 부모 클래스.
  - BaseCube: Cube 형태의 객체의 가장 기본이 되는 부모 클래스. 회전하는 Rotation Cube 객체는 이 클래스를 상속받음.
  - Model: 텍스쳐 파일을 가지는 객체 클래스. Renderable 클래스를 상속받아 mesh, material 등을 사용함.
  - Character: 키보드 입력을 받아 이동 및 회전 기능을 가지는 객체 클래스. Model 클래스를 상속받아 골렘 캐릭터를 생성.


# 함수 소개

- InitDevice: GPU Pipeline을 위한 Resource 생성
  - Render Target, Back Buffer, Depth Buffer, ViewPort
  - Vertex Buffer, Index Buffer
  - Vertex Shader, Pixel Shader, Input Layout 등
 

  <br>
  
  - 시간 Frame 당 회전 Rotation Matrix 계산
      <img width="1266" height="514" alt="image" src="https://github.com/user-attachments/assets/670504f5-8277-4034-a0f4-b4acea230e72" />

  
- Shader.fx: Vertex Shader, Pixel Shader 구현
  
  <img width="800" height="254" alt="image" src="https://github.com/user-attachments/assets/d78aac2a-6abc-4595-ad4a-60e0a8b320a3" />
  <img width="800" height="344" alt="image" src="https://github.com/user-attachments/assets/158bb10c-e6df-4878-9df8-823c7bccd912" />


- 플레이어의 입력을 받아 골렘의 vertex들을 움직여주었고 그에 따라 카메라의 시점도 함께 바꾸어주었습니다.
  
<img width="387" height="299" alt="image" src="https://github.com/user-attachments/assets/07f31be1-59ff-4940-838f-595b2c3a0089" />
<img width="362" height="244" alt="image" src="https://github.com/user-attachments/assets/db61176e-d7e4-4206-b50a-2cf8da3a6d94" />
