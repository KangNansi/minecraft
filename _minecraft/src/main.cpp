//Includes application
#include <conio.h>
#include <vector>
#include <string>
#include <windows.h>
#include <ctime>

#include "external/gl/glew.h"
#include "external/gl/freeglut.h"
#include "world.h"
#include "avatar.h"
#include "my_physics.h"

//Moteur
#include "engine/utils/types_3d.h"
#include "engine/timer.h"
#include "engine/log/log_console.h"
#include "engine/render/renderer.h"
#include "engine/gui/screen.h"
#include "engine/gui/screen_manager.h"



NYRenderer * g_renderer = NULL;
NYTimer * g_timer = NULL;
int g_nb_frames = 0;
float g_elapsed_fps = 0;
int g_main_window_id;
int g_mouse_btn_gui_state = 0;
bool g_fullscreen = false;
GLuint g_shader = 0;

//GUI 
GUIScreenManager * g_screen_manager = NULL;
GUIBouton * BtnParams = NULL;
GUIBouton * BtnClose = NULL;
GUILabel * LabelFps = NULL;
GUILabel * LabelCam = NULL;
GUIScreen * g_screen_params = NULL;
GUIScreen * g_screen_jeu = NULL;
GUISlider * g_slider;

//Camera controls
bool camera_mode = false;
float time_offset = 0.0f;
float moment = 0.0f;
float _momentSpeed = 0.01f;
float _dayBegin = 0.25;
float _dayEnd = 0.75;

//World
NYWorld * g_world;

NYAvatar * player;

NYVert3Df lightPos;
void setSkyColor();

//////////////////////////////////////////////////////////////////////////
// GESTION APPLICATION
//////////////////////////////////////////////////////////////////////////
void update(void)
{
	float elapsed = g_timer->getElapsedSeconds(true);

	static float g_eval_elapsed = 0;

	//Calcul des fps
	g_elapsed_fps += elapsed;
	g_nb_frames++;
	if(g_elapsed_fps > 1.0)
	{
		LabelFps->Text = std::string("FPS : ") + toString(g_nb_frames);
		g_elapsed_fps -= 1.0f;
		g_nb_frames = 0;

		//Affichage position de la camera
		LabelCam->Text = std::string("Cam[Position] : " 
			+ toString(g_renderer->_Camera->_Position.X)
			+ " " + toString(g_renderer->_Camera->_Position.Y)
			+ " " + toString(g_renderer->_Camera->_Position.Z)
			+ "\nCam[Direction] : " 
			+ toString(g_renderer->_Camera->_Direction.X)
			+ " " + toString(g_renderer->_Camera->_Direction.Y)
			+ " " + toString(g_renderer->_Camera->_Direction.Z));
	}

	moment += elapsed*_momentSpeed;
	if (moment > 1.0)
		moment -= (int)moment;
	
	setSkyColor();
	//Physique
	player->update(elapsed);
	if (!camera_mode)
		g_renderer->_Camera->moveTo(player->Position+NYVert3Df(player->Width/2, player->Width/2, player->Height*0.8) - g_renderer->_Camera->_Direction*0.5);
	else {
		g_renderer->_Camera->setLookAt(player->Position);
		g_renderer->_Camera->moveTo(player->Position + NYVert3Df(-5, 0, 5));
	}

	//Rendu
	g_renderer->render(elapsed);
}

void setSkyColor() {
	NYColor background(0.7, 0.3, 0.3, 1.0);
	float alpha = sin(moment*2*M_PI - (M_PI/2.));
	if (alpha < 0) {
		background = background.interpolate(NYColor(0.1, 0.1, 0.2, 1.0), -alpha);
	}
	else
		background = background.interpolate(NYColor(0.8, 0.8, 1.0, 1.0), alpha);

	g_renderer->setBackgroundColor(background);

}

void render2d(void)
{
	g_screen_manager->render();
}

void setLights(void)
{
	//On active la light 0
	glEnable(GL_LIGHT0);

	//On d�finit une lumi�re directionelle (un soleil)

	float direction[4] = {0,0,-1,0}; ///w = 0 donc elle est a l'infini
	glLightfv(GL_LIGHT0, GL_POSITION, direction );
	float color[4] = {0.5f,0.5f,0.5f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, color );
	float color2[4] = {0.1f,0.1f,0.1f};
	glLightfv(GL_LIGHT0, GL_AMBIENT, color2 );
	float color3[4] = {0.3f,0.3f,0.3f};
	glLightfv(GL_LIGHT0, GL_SPECULAR, color3 );
}

void renderObjects(void)
{

	//Rendu des axes
	glDisable(GL_LIGHTING);

	glBegin(GL_LINES);
	glColor3d(1,0,0);
	glVertex3d(0,0,0);
	glVertex3d(10000,0,0);
	glColor3d(0,1,0);
	glVertex3d(0,0,0);
	glVertex3d(0,10000,0);
	glColor3d(0,0,1);
	glVertex3d(0,0,0);
	glVertex3d(0,0,10000);
	glEnd();

	NYVert3Df point;
	NYVert3Df cube;
	if (g_world->interDroiteMatrice(g_renderer->_Camera->_Position, g_renderer->_Camera->_Direction*10, point, cube)) {
		glPushMatrix();
		glTranslatef(point.X, point.Y, point.Z);
		glColor3f(1, 0, 0);
		glutSolidCube(0.2);
		glPopMatrix();
	}

	glEnable(GL_LIGHTING);

	glPushMatrix();
	glRotatef((moment+time_offset) * 360 , 1, 0, 0);
	glTranslatef(0, 0, -1000);
	NYVert3Df pos_cam = g_renderer->_Camera->_Position;
	glTranslatef(pos_cam.X, pos_cam.Y, pos_cam.Z);
	GLfloat emissive[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_EMISSION, emissive);
	glutSolidSphere(20, 20, 20);
	setLights();
	glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);

	glPopMatrix();

	glPushMatrix();
	glTranslatef(player->Position.X, player->Position.Y, player->Position.Z);
	player->render();
	glPopMatrix();

	glUseProgram(g_shader);

	GLuint elap = glGetUniformLocation(g_shader, "elapsed");
	glUniform1f(elap, NYRenderer::_DeltaTimeCumul);

	GLuint amb = glGetUniformLocation(g_shader, "ambientLevel");
	glUniform1f(amb, 0.4);

	NYFloatMatrix invertView;
	invertView.createViewMatrix(g_renderer->_Camera->_Position, g_renderer->_Camera->_LookAt, g_renderer->_Camera->_UpVec);
	invertView.invert();
	GLuint invView = glGetUniformLocation(g_shader, "invertView");
	glUniformMatrix4fv(invView, 1, true, invertView.Mat.t);


	//Affichage du monde
	glPushMatrix();
	g_world->render_world_vbo();
	glPopMatrix();

	

}

void resizeFunction(int width, int height)
{
	glViewport(0, 0, width, height);
	g_renderer->resize(width,height);
}

//////////////////////////////////////////////////////////////////////////
// GESTION CLAVIER SOURIS
//////////////////////////////////////////////////////////////////////////

void specialDownFunction(int key, int p1, int p2)
{
	//On change de mode de camera
	if(key == GLUT_KEY_LEFT)
	{
		camera_mode = !camera_mode;
	}

}

void specialUpFunction(int key, int p1, int p2)
{

}

void keyboardDownFunction(unsigned char key, int p1, int p2)
{
	if(key == VK_ESCAPE)
	{
		glutDestroyWindow(g_main_window_id);	
		exit(0);
	}

	if(key == 'f')
	{
		if(!g_fullscreen){
			glutFullScreen();
			g_fullscreen = true;
		} else if(g_fullscreen){
			glutLeaveGameMode();
			glutLeaveFullScreen();
			glutReshapeWindow(g_renderer->_ScreenWidth, g_renderer->_ScreenWidth);
			glutPositionWindow(0,0);
			g_fullscreen = false;
		}
	}

	//Mouvement joueur
	if (key == 'z') {
		player->avance = true;
	}
	if (key == 'q') {
		player->gauche = true;
	}
	if (key == 's') {
		player->recule = true;
	}
	if (key == 'd') {
		player->droite = true;
	}
	if (key == VK_SPACE)
		player->Jump = true;
	if (key == 'e') {
		NYVert3Df point;
		NYVert3Df cube;
		if (g_world->interDroiteMatrice(g_renderer->_Camera->_Position, g_renderer->_Camera->_Direction * 10, point, cube))
			g_world->deleteCube(cube.X, cube.Y, cube.Z);
	}

	if (key == 't') {
		g_shader = g_renderer->createProgram("shaders/psbase.glsl", "shaders/vsbase.glsl");
	}
		

	//Controle du temps
	if (key == 'g') {
		time_offset += 0.01f;
	}
	if (key == 'h') {
		time_offset -= 0.01f;
	}
	if (key == 'j') {
		_momentSpeed += 0.01f;
	}
	if (key == 'k') {
		_momentSpeed -= 0.01f;
	}

	if (key == 'c') {
		camera_mode = !camera_mode;
	}
}

void keyboardUpFunction(unsigned char key, int p1, int p2)
{
	//Mouvement joueur
	if (key == 'z') {
		player->avance = false;
	}
	if (key == 'q') {
		player->gauche = false;
	}
	if (key == 's') {
		player->recule = false;
	}
	if (key == 'd') {
		player->droite = false;
	}
	if (key == VK_SPACE)
		player->Jump = false;
}

void mouseWheelFunction(int wheel, int dir, int x, int y)
{
	g_renderer->_Camera->moveForward(dir);
}

void mouseFunction(int button, int state, int x, int y)
{
	//Gestion de la roulette de la souris
	if((button & 0x07) == 3 && state)
		mouseWheelFunction(button,1,x,y);
	if((button & 0x07) == 4 && state)
		mouseWheelFunction(button,-1,x,y);

	//GUI
	g_mouse_btn_gui_state = 0;
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		g_mouse_btn_gui_state |= GUI_MLBUTTON;
	
	bool mouseTraite = false;
	mouseTraite = g_screen_manager->mouseCallback(x,y,g_mouse_btn_gui_state,0,0);
}

void mouseMoveFunction(int x, int y, bool pressed)
{
	static int last_x = x;
	static int last_y = y;
	bool mouseTraite = false;

	mouseTraite = g_screen_manager->mouseCallback(x,y,g_mouse_btn_gui_state,0,0);
	if(pressed && mouseTraite)
	{
		//Mise a jour des variables li�es aux sliders
	}
	else if (pressed) {
		glutSetCursor(GLUT_CURSOR_NONE);
		if (glutGetModifiers() & GLUT_ACTIVE_SHIFT != 0) {
			g_renderer->_Camera->rotateAround(-(x-last_x)/40.0f);
			g_renderer->_Camera->rotateUpAround((y - last_y) / 40.0f);
		}
		else {
			g_renderer->_Camera->rotate(-(x - last_x) / 40.0f);
			g_renderer->_Camera->rotateUp((y - last_y) / 40.0f);
		}
	}
	else {
		glutSetCursor(GLUT_CURSOR_BOTTOM_SIDE);
	}

	last_x = x;
	last_y = y;
}

void mouseMoveActiveFunction(int x, int y)
{
	mouseMoveFunction(x,y,true);
}
void mouseMovePassiveFunction(int x, int y)
{
	mouseMoveFunction(x,y,false);
}


void clickBtnParams (GUIBouton * bouton)
{
	g_screen_manager->setActiveScreen(g_screen_params);
}

void clickBtnCloseParam (GUIBouton * bouton)
{
	g_screen_manager->setActiveScreen(g_screen_jeu);
}

/**
  * POINT D'ENTREE PRINCIPAL
  **/
int main(int argc, char* argv[])
{ 
	LogConsole::createInstance();

	int screen_width = 800;
	int screen_height = 600;

	glutInit(&argc, argv); 
	glutInitContextVersion(3,0);
	glutSetOption(
		GLUT_ACTION_ON_WINDOW_CLOSE,
		GLUT_ACTION_GLUTMAINLOOP_RETURNS
		);

	glutInitWindowSize(screen_width,screen_height);
	glutInitWindowPosition (0, 0);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE );

	glEnable(GL_MULTISAMPLE);

	Log::log(Log::ENGINE_INFO, (toString(argc) + " arguments en ligne de commande.").c_str());	
	bool gameMode = true;
	for(int i=0;i<argc;i++)
	{
		if(argv[i][0] == 'w')
		{
			Log::log(Log::ENGINE_INFO,"Arg w mode fenetre.\n");
			gameMode = false;
		}
	}

	if(gameMode)
	{
		int width = glutGet(GLUT_SCREEN_WIDTH);
		int height = glutGet(GLUT_SCREEN_HEIGHT);
		
		char gameModeStr[200];
		sprintf(gameModeStr,"%dx%d:32@60",width,height);
		glutGameModeString(gameModeStr);
		g_main_window_id = glutEnterGameMode();
	}
	else
	{
		g_main_window_id = glutCreateWindow("MyNecraft");
		glutReshapeWindow(screen_width,screen_height);
	}

	if(g_main_window_id < 1) 
	{
		Log::log(Log::ENGINE_ERROR,"Erreur creation de la fenetre.");
		exit(EXIT_FAILURE);
	}
	
	GLenum glewInitResult = glewInit();

	if (glewInitResult != GLEW_OK)
	{
		Log::log(Log::ENGINE_ERROR,("Erreur init glew " + std::string((char*)glewGetErrorString(glewInitResult))).c_str());
		_cprintf("ERROR : %s",glewGetErrorString(glewInitResult));
		exit(EXIT_FAILURE);
	}

	//Affichage des capacit�s du syst�me
	Log::log(Log::ENGINE_INFO,("OpenGL Version : " + std::string((char*)glGetString(GL_VERSION))).c_str());

	glutDisplayFunc(update);
	glutReshapeFunc(resizeFunction);
	glutKeyboardFunc(keyboardDownFunction);
	glutKeyboardUpFunc(keyboardUpFunction);
	glutSpecialFunc(specialDownFunction);
	glutSpecialUpFunc(specialUpFunction);
	glutMouseFunc(mouseFunction);
	glutMotionFunc(mouseMoveActiveFunction);
	glutPassiveMotionFunc(mouseMovePassiveFunction);
	glutIgnoreKeyRepeat(1);

	//Initialisation du renderer
	g_renderer = NYRenderer::getInstance();
	g_renderer->setRenderObjectFun(renderObjects);
	g_renderer->setRender2DFun(render2d);
	g_renderer->setLightsFun(setLights);
	g_renderer->setBackgroundColor(NYColor());
	g_renderer->initialise(true);

	g_shader = g_renderer->createProgram("shaders/psbase.glsl", "shaders/vsbase.glsl");

	//On applique la config du renderer
	glViewport(0, 0, g_renderer->_ScreenWidth, g_renderer->_ScreenHeight);
	g_renderer->resize(g_renderer->_ScreenWidth,g_renderer->_ScreenHeight);
	
	//Ecran de jeu
	uint16 x = 10;
	uint16 y = 10;
	g_screen_jeu = new GUIScreen(); 

	g_screen_manager = new GUIScreenManager();
		
	//Bouton pour afficher les params
	BtnParams = new GUIBouton();
	BtnParams->Titre = std::string("Params");
	BtnParams->X = x;
	BtnParams->setOnClick(clickBtnParams);
	g_screen_jeu->addElement(BtnParams);

	y += BtnParams->Height + 1;

	LabelFps = new GUILabel();
	LabelFps->Text = "FPS";
	LabelFps->X = x;
	LabelFps->Y = y;
	LabelFps->Visible = true;
	g_screen_jeu->addElement(LabelFps);

	y += LabelFps->Height + 1;

	//Camera
	LabelCam = new GUILabel();
	LabelCam->Text = "Pos";
	LabelCam->X = x;
	LabelCam->Y = y;
	LabelCam->Visible = true;
	g_screen_jeu->addElement(LabelCam);

	//Ecran de parametrage
	x = 10;
	y = 10;
	g_screen_params = new GUIScreen();

	GUIBouton * btnClose = new GUIBouton();
	btnClose->Titre = std::string("Close");
	btnClose->X = x;
	btnClose->setOnClick(clickBtnCloseParam);
	g_screen_params->addElement(btnClose);

	y += btnClose->Height + 1;
	y+=10;
	x+=10;

	GUILabel * label = new GUILabel();
	label->X = x;
	label->Y = y;
	label->Text = "Param :";
	g_screen_params->addElement(label);

	y += label->Height + 1;

	g_slider = new GUISlider();
	g_slider->setPos(x,y);
	g_slider->setMaxMin(1,0);
	g_slider->Visible = true;
	g_screen_params->addElement(g_slider);

	y += g_slider->Height + 1;
	y+=10;

	//Ecran a rendre
	g_screen_manager->setActiveScreen(g_screen_jeu);
	
	//Init Camera
	g_renderer->_Camera->setPosition(NYVert3Df(10,10,10));
	g_renderer->_Camera->setLookAt(NYVert3Df(0,0,0));
	

	//Fin init moteur

	//Init application

	//Init day time
	std::time_t time = std::time(nullptr);
	std::tm *local_time = std::localtime(&time);
	float moment = local_time->tm_sec + local_time->tm_min * 60 + local_time->tm_hour * 3600;
	moment /= (24 * 3600);
	moment += time_offset;

	

	//Init Timer
	g_timer = new NYTimer();
	
	//On start
	g_timer->start();

	//Cr�ation du monde
	g_world = new NYWorld();
	g_world->_FacteurGeneration = 5;
	g_world->init_world();

	//Init avatar
	player = new NYAvatar(g_renderer->_Camera, g_world);
	player->Position = NYVert3Df(30, 30, 100);
	g_renderer->_Camera->_Position = player->Position;

	glutMainLoop(); 

	return 0;
}

