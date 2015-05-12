#include "mainwindow.h"
#include "fileloader.h"
#include "global.h"
#include "rule.h"
#include "octree.h"
#include "text2d.h"

#include <ctime>
#include <cstdlib>

using namespace ork;

static_ptr<Window> MainWindow::app;
int MainWindow::GameState = 0;//比赛状态 -1失败 0正在进行 1胜利

Camera *camera = NULL;//摄像机
ptr<FrameBuffer> screenFb = NULL;//帧缓存
ptr<FrameBuffer> depthFb = NULL;//帧缓存
ptr<TextureCube> skyboxTex = NULL;//天空盒
ptr<TextureCube> glassTex = NULL;//天空盒 for glass
Light light1;//一个全局灯光
Light light2;//灯光2
ptr<Texture2D> depthTexture = NULL;//深度贴图
ptr<Texture2D> colorTexture = NULL;//颜色贴图
vec2f ScreenSize = vec2f(WIDTH, HEIGHT);//屏幕大小
float myTimer = 0;//计时器
int whichTurn = -1;//轮到谁下 0 Ally 1 Rival  -1 未确定
//比例方面的变量
float ratioX;
float ratioY;
float ratio;
vec2f ratioXY;

MainWindow::MainWindow() : GlutWindow(Window::Parameters().name("Chess 3D").size(WIDTH, HEIGHT))
{
	InitCamera();
	InitLight();
	InitFBO();

	InitSkybox();
	InitModel();
	InitPlayer();

	//初始化2D文字 图标
	Init2D();

	//初始化网络
	ServerClient::Init();
	ServerClient::CreateThreads();

	//设置随机数的种子
	srand((int)time(NULL));
}

MainWindow::~MainWindow()
{
	delete ally;
	delete rival;
	delete axes;
	delete clock;

	delete Board::GetInstance();

	ServerClient::WaitThreads();
	ServerClient::Close();
}

//重绘函数 t:重绘时刻 dt:上一次重绘时间间隔
void MainWindow::redisplay(double t, double dt)
{
	dt *= TIME_RATIO;

	ratioX = ScreenSize.x / WIDTH;
	ratioY = ScreenSize.y / HEIGHT;
	ratio = sqrt(ratioX * ratioY);
	ratioXY = vec2f(ratioX, ratioY); 

	//初始化屏幕frame buffer
	screenFb->setClearColor(vec4f(0, 0, 0, 1.0));
	screenFb->clear(true, false, true);
	screenFb->setDepthTest(true);
	//初始化深度frame buffer
	depthFb->setClearColor(vec4f(0, 0, 0, 1.0));
	depthFb->clear(true, false, true);
	depthFb->setDepthTest(true);

	if (showHelp)
	{
		helpImg->Draw(0, HEIGHT);
	}
	else if (showAbout)
	{
		aboutImg->Draw(0, HEIGHT);
	}
	else
	{
		if (GameState == 0)
		{
			//更新键盘事件
			UpdateKey();
			//更新按钮
			UpdateButton();
		}
		
		//更新摄像机矩阵
		vec4i viewport = screenFb->getViewport();
		camera->Update(viewport.z, viewport.w);
		ScreenSize = vec2f(viewport.z, viewport.w);
		//如果是绘制到屏幕上的framebuffer，画天空盒
		screenFb->setDepthMask(false);
		skyboxProgram->getUniformMatrix4f("screenToModelMat")->setMatrix(camera->VP.inverse());
		skyboxProgram->getUniform3f("cameraPos")->set(camera->pos);
		screenFb->draw(skyboxProgram, *skyboxMesh);
		screenFb->setDepthMask(true);

		//绘制场景
		//Model::SetRenderDepth(true);//开启绘制深度贴图
		//RenderScene(depthFb, dt);

		//Model::SetRenderDepth(false);//关闭绘制深度贴图
		RenderScene(screenFb, dt);

		//如果是绘制到屏幕上的framebuffer，画坐标轴
		axes->Render(screenFb);

		//plane->SetDiffuseTexture(colorTexture);
		//plane->Render(screenFb);

		Draw2D(dt);//写文字 画图标

		if(GameState == -1)
		{
			loseImg->Draw(0, HEIGHT);
		}
		else if(GameState == 1)
		{
			winImg->Draw(0, HEIGHT);
		}
	}
	GlutWindow::redisplay(t, dt);
}

//窗口大小变化调用函数
void MainWindow::reshape(int x, int y)
{
	FrameBuffer::getDefault()->setViewport(vec4<GLint>(0, 0, x, y));
	halfW = x / 2;
	halfH = y / 2;
	GlutWindow::reshape(x, y);
	idle(false);
}

//键盘响应
bool MainWindow::keyTyped(unsigned char c, EventHandler::modifier m, int x, int y)
{
	if (c == 27) {
		//::exit(0);
		showHelp = showSanti = showAbout = false;
	}
	if (c == 'f' || c == 'F'){
	}
	return true;
}

//更新键盘事件
void MainWindow::UpdateKey()
{
	if (GetAsyncKeyState('W')){
		camera->Slide(0, 0, -0.1);
	}
	if (GetAsyncKeyState('S')){
		camera->Slide(0, 0, 0.1);
	}
	if (GetAsyncKeyState('A')){
		camera->Slide(-0.1, 0, 0);
	}
	if (GetAsyncKeyState('D')){
		camera->Slide(0.1, 0, 0);
	}
	if (GetAsyncKeyState('R')){
		camera->Reset();
	}
}

//更新按钮
void MainWindow::UpdateButton()
{
	if (helpBtn->isClicked)
	{
		HelpBtnSlot();
		helpBtn->isClicked = false;
	}

	if (santiBtn->isClicked)
	{
		SantiBtnSlot();
		santiBtn->isClicked = false;
	}

	if (aboutBtn->isClicked)
	{
		AboutBtnSlot();
		aboutBtn->isClicked = false;
	}
}

//按钮槽函数
void MainWindow::HelpBtnSlot()
{
	showHelp = true;
}

void MainWindow::SantiBtnSlot()
{
	showSanti = true;
}

void MainWindow::AboutBtnSlot()
{
	showAbout = true;
}

bool MainWindow::mouseMotion(int x, int y)
{
	if (GameState == 0)
	{
		float dx, dy;

		x -= halfW;
		y -= halfH;

		dx = (float)(x - oldX) * camera->mouseSensitivity;
		dy = (float)(y - oldY) * camera->mouseSensitivity;

		oldX = x;
		oldY = y;

		camera->RotateX(dx);
		camera->RotateY(dy);
	}
	return true;
}

bool MainWindow::mousePassiveMotion(int x, int y)
{
	helpBtn->MouseMove(x, y);
	santiBtn->MouseMove(x, y);
	aboutBtn->MouseMove(x, y);

	return true;
}

bool MainWindow::mouseClick(EventHandler::button b, EventHandler::state s, EventHandler::modifier m, int x, int y)
{
	oldX = x - halfW;
	oldY = y - halfH;

	if (whichTurn == 0)//只有轮到我方时才能下
	{
		ally->RayCast(x, y, s == UP ? 0 : 1);
	}
	
	if(s == 0)
	{
		helpBtn->MousePress(x, y);
		santiBtn->MousePress(x, y);
		aboutBtn->MousePress(x, y);
	}
	else if(s == 1)
	{
		helpBtn->MouseRelease(x, y);
		santiBtn->MouseRelease(x, y);
		aboutBtn->MouseRelease(x, y);
	}

	idle(false);
	return true;
}

bool MainWindow::mouseWheel(EventHandler::wheel b, EventHandler::modifier m, int x, int y)
{
	printf("wheel!\n");
	return true;
}

void MainWindow::RenderScene(ptr<FrameBuffer> fb, float dt)//画场景
{
	ally->Logic(dt);
	ally->Render(fb, dt);

	rival->Logic(dt);
	rival->Render(fb, dt);
}

void MainWindow::Draw2D(float dt)//写文字 图画标
{
	Text2D::Print(string("Ally: ") + ally->GetName() + " " + ally->GetOffsensiveStr(), 80, 700, 40, vec3f(0.0f, 1.0f, 0.0f));
	Text2D::Print(string("Rival: ") + rival->GetName() + " " + rival->GetOffsensiveStr(), 80, 660, 40, vec3f(1.0f, 0.0f, 0.0f));

	myTimer += dt;

	if(whichTurn == 0)
	{
		Text2D::Print("It's your turn.", 670, 720, 35, vec3f(0.0f, 1.0f, 0.0f));
	}
	else if(whichTurn == 1)
	{
		Text2D::Print("It's rival's turn.", 670, 720, 35, vec3f(1.0f, 0.0f, 0.0f));
	}
	else if(whichTurn == -1)
	{
		Text2D::Print("Link others or wait to be linked.", 670, 720, 35, vec3f(0.0f, 0.0f, 1.0f));
	}

	Text2D::Print(GetHMSFormatStr(myTimer).c_str(), 820, 650, 35, vec3f(1.0f, 1.0f, 0.0f));

	textBgImgLeft->Draw(72, 128);
	textBgImgRight->Draw(680, 128);
	clock->Draw(760, 118);

	helpBtn->Render();
	santiBtn->Render();
	aboutBtn->Render();
}

//初始化摄像机
void MainWindow::InitCamera()
{
	camera = Camera::GetCamera();
}

//初始化灯光
void MainWindow::InitLight()
{
	light1.pos = vec3f(0.0f, 4.0f, 0.0f);
	light1.color = vec3f(1.0f, 1.0f, 1.0f);
	light1.dir = vec3f(1.0f, 1.0f, 1.0f);
	light1.power = 6.0f;
	light1.lightType = SpotLight;//其实是PointLight，只是SpotLight比较好做ShadowMap而已

	light2.pos = vec3f(0.0f, 4.0f, -3.0f);
	light2.color = vec3f(1.0f, 1.0f, 1.0f);
	light1.dir = vec3f(0.5f, 2, 2);
	light2.power = 15.0f;
	light2.lightType = SpotLight;//其实是PointLight，只是SpotLight比较好做ShadowMap而已
}

//初始化FBO
void MainWindow::InitFBO()
{
	screenFb = FrameBuffer::getDefault();
	depthFb = new FrameBuffer();
	depthFb->setViewport(vec4i(0, 0, 1024, 1024));
}

//初始化天空盒
void MainWindow::InitSkybox()
{
	skyboxMesh = new Mesh<vec4f, unsigned int>(TRIANGLE_STRIP, GPU_STATIC);
	skyboxMesh->addAttributeType(0, 4, A32F, false);

	skyboxMesh->addVertex(vec4f(-1, -1, 0, 0));
	skyboxMesh->addVertex(vec4f(1, -1, 1, 0));
	skyboxMesh->addVertex(vec4f(-1, 1, 0, 1));
	skyboxMesh->addVertex(vec4f(1, 1, 1, 1));

	skyboxTex = FileLoader::LoadTextureCube(GetTexturePath("skybox/FishermansBastion"));
	glassTex = FileLoader::LoadTextureCube(GetTexturePath("skybox/Green"));

	skyboxProgram = FileLoader::LoadProgram(GetShaderPath("SkyBox.glsl"));

	skyboxProgram->getUniformSampler("skyboxTex")->set(skyboxTex);
}

//初始化模型
void MainWindow::InitModel()
{
	printf("---------Load Scene Stuff--------\n");

	axes = new Model("axes.obj", false, true);
	//earth = new Model("earth.obj");
	//plane = new Model("plane.obj");

	//初始化深度贴图
	Texture::Parameters DepthTexturePara = Texture::Parameters().min(LINEAR).mag(LINEAR).
		wrapS(CLAMP_TO_EDGE).wrapT(CLAMP_TO_EDGE).
		compareFunc(LEQUAL);

	depthTexture = new Texture2D(1024, 1024, TextureInternalFormat::DEPTH_COMPONENT32F, DEPTH_COMPONENT, PixelType::FLOAT, 
		DepthTexturePara, Buffer::Parameters(), CPUBuffer());

	//初始化颜色贴图
	Texture::Parameters ColorTexturePara = Texture::Parameters().min(LINEAR).mag(LINEAR).
		wrapS(CLAMP_TO_EDGE).wrapT(CLAMP_TO_EDGE).compareFunc(LEQUAL);

	colorTexture = new Texture2D(1024, 1024, R32F, RED, PixelType::FLOAT, ColorTexturePara, Buffer::Parameters(), CPUBuffer());

	depthFb->setDrawBuffer(BufferId(DEPTH | COLOR0));
	depthFb->setTextureBuffer(DEPTH, depthTexture, 0);
	depthFb->setTextureBuffer(COLOR0, colorTexture, 0);
}

//初始化玩家
void MainWindow::InitPlayer()
{
	ally = Ally::GetInstance();
	ally->Init();

	rival = Rival::GetInstance();
	rival->Init();

	Rule::board = Board::GetInstance();
	Rule::ally = Ally::GetInstance();
	Rule::rival = Rival::GetInstance();

	OctreeManager::Init();//初始化OctreeManager
}

//初始化2D文字 图标
void MainWindow::Init2D()
{
	showHelp = showSanti = showAbout = false;
	//初始化text2D
	Text2D::Init("Consolas.png");
	//初始化overlay2D
	clock = new Overlay2D("clock.png", vec2f(0.8f, 0.8f));
	textBgImgLeft = new Overlay2D("TextBgImg.png");
	textBgImgRight = new Overlay2D("TextBgImg.png", vec2f(1.02f, 1.0f));

	helpBtn = new Button("help_common.png", "help_hover.png", "help_press.png", vec2i(20, 235));
	santiBtn = new Button("santi_common.png", "santi_hover.png", "santi_press.png", vec2i(20, 290));
	aboutBtn = new Button("about_common.png", "about_hover.png", "about_press.png", vec2i(20, 345));

	helpImg = new Overlay2D("help.png");
	aboutImg = new Overlay2D("about.png");
	winImg = new Overlay2D("win.png");
	loseImg = new Overlay2D("lose.png");
}

string MainWindow::GetHMSFormatStr(float t)//得到时-分-秒标准时间格式字符串
{
	int h = t / 3600;
	int m = (t - 3600 * h) / 60;
	int s = t - 3600 * h - 60 * m;

	std::string strH, strM, strS;
	char buffer[5];

	if (h == 0)
	{
		strH = "00";
	}
	else if (h < 10)
	{
		strH = std::string("0") + itoa(h, buffer, 10);
	}
	else
	{
		strH = itoa(h, buffer, 10);
	}

	if (m == 0)
	{
		strM = "00";
	}
	else if (m < 10)
	{
		strM = std::string("0") + itoa(m, buffer, 10);
	}
	else
	{
		strM = itoa(m, buffer, 10);
	}

	if (s == 0)
	{
		strS = "00";
	}
	else if (s < 10)
	{
		strS = std::string("0") + itoa(s, buffer, 10);
	}
	else
	{
		strS = itoa(s, buffer, 10);
	}

	return strH + ":" + strM + ":" + strS;
}