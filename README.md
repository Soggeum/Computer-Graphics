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

---

#  Game
  
<img width="646" height="327" alt="image" src="https://github.com/user-attachments/assets/c7dfd309-0153-4818-9281-25ae0400185a" />
   
- Main Window와 Render 객체를 통해 게임을 진행합니다.
- Window에서 받은 입력을 처리하거나 입력이 없을 경우 Render 객체를 이용하여 렌더링합니다.

<img width="800" height="450" alt="image" src="https://github.com/user-attachments/assets/29a74e4c-0e6a-490c-b13e-1f837ab6977a" />

---

# Renderer

<img width="1347" height="464" alt="image" src="https://github.com/user-attachments/assets/f7ae8639-1265-44d9-b144-22b516efe4f2" />
  
- 렌더링을 담당하는 객체로, 렌더링 후 화면에 표시까지 해줍니다.
  
- color/depth buffer 초기화 (ClearRenderTargetView, ClearDepthStencilView)
  
- constant buffer update
  
- Pipeline state(셰이더, 버퍼 등) 설정 및 binding
 
- Draw로 오브젝트 렌더링
  
- present로 front/back buffer swap하여 화면에 출력
  
---

# Renderable
  
- 재사용 가능한 객체 Renderable, BaseCube, Model, Character를 구현하여 렌더링하였습니다.
<img width="661" height="444" alt="image" src="https://github.com/user-attachments/assets/fd65f5d7-37f8-441c-95f7-72add46b4762" />
 
- Renderable: 렌더링 되어질 객체들의 가장 기본, 부모 클래스입니다.
  <img width="555" height="485" alt="image" src="https://github.com/user-attachments/assets/b2a12983-90cc-44ae-9190-5bbd4df7b43a" />

   - BasicMeshEntry 구조체를 이용하여 mesh 정보를 저장하였습니다.
       <img width="1096" height="500" alt="image" src="https://github.com/user-attachments/assets/c270f112-3efb-466c-a42e-e9d8a9f5c8d4" />
   - Material class를 이용하여 텍스쳐 정보와 색상 정보를 저장하였습니다.
       <img width="618" height="333" alt="image" src="https://github.com/user-attachments/assets/f94fc28a-1916-4828-b61a-460aaa2d532b" />


- BaseCube: Cube 형태 객체의 가장 기본이 되는 부모 클래스입니다.
  
- Model: 텍스쳐 파일을 가지는 객체 클래스입니다.
  
- Character: 키보드 입력을 받아 이동 및 회전 기능을 가지는 객체 클래스로 이 클래스를 사용하여 골렘을 만들었습니다.

---

# Shader

<img width="470" height="276" alt="image" src="https://github.com/user-attachments/assets/c384cc3a-bf13-40da-a21c-4e8ef10fdabd" />

- Vertex Shader, Pixel Shader를 .fx파일을 이용하여 구현하였습니다.
- Vertex Shader: 각 Vertex에 World, View, Projection Transform을 해주었습니다.
  <img width="1001" height="512" alt="image" src="https://github.com/user-attachments/assets/47676a0b-08a3-4937-8760-967b36f5c733" />

- Pixel Shader: Phong Lighting Model을 이용하여 색상을 결정해주었습니다. 광원의 경우(PSSolid) 처리 없이 바로 색을 반환했습니다.
  <img width="1818" height="863" alt="image" src="https://github.com/user-attachments/assets/3fd7de42-53c7-4ab6-9861-db2b723324d6" />


# 키보드 입력에 따른 골렘 및 카메라 움직임

<img width="600" height="300" alt="image" src="https://github.com/user-attachments/assets/07f31be1-59ff-4940-838f-595b2c3a0089" />

<img width="600" height="300" alt="image" src="https://github.com/user-attachments/assets/db61176e-d7e4-4206-b50a-2cf8da3a6d94" />
