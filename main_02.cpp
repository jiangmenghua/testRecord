#include <math.h>
#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <string>


// angle of rotation for the camera direction
float angle = 0.0f;
// vector representing camera direction
float lx = 0.0f, ly = 0.0f, lz = -1.0f;
// xz position of the camera
float cam_x = 1.0f, cam_z = 5.0f;
// xz position of the observe point
float ob_x = 0.0f, ob_z = 0.0f;

// key states, variables are zeros when no keys are pressed
float deltaAngle = 0.0f, deltaMoveWS = 0, deltaMoveAD = 0;


namespace GUI {
	typedef struct {
		float pt_x, pt_y, pt_z;
	}POINT;

	typedef struct {
		POINT			astPoint[256];
		DWORD		dwNumOfPoints;
	}POINTS;
}
GUI::POINTS stPoints = { 0 };


void changeSize(int, int);
void computePos(float, float);
void computeDir(float);
void renderScene();
void pressSpecialKey(int, int, int);
void releaseSpecialKey(int, int, int);
void drawAxes();
void drawGrid();
void drawSphere(GUI::POINTS*);
void pressNormalKey(unsigned char, int, int);
void releaseNormalKey(unsigned char, int, int);
void parsePoints(char* pch, GUI::POINTS*);


int main(int argc, char **argv)
{
	
	char chFileName[] = "points.txt";
	parsePoints(chFileName, &stPoints);


	// init glut and create window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(640, 640);
	glutCreateWindow("Test camera move");

	// register call backs
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	glutIdleFunc(renderScene);
	glutKeyboardFunc(pressNormalKey);
	glutSpecialFunc(pressSpecialKey);

	// new entries
	glutIgnoreKeyRepeat(1);
	glutSpecialUpFunc(releaseSpecialKey);
	glutKeyboardUpFunc(releaseNormalKey);

	// opengl init
	glEnable(GL_DEPTH_TEST);

	// main loop
	glutMainLoop();

	return 0;
}


void changeSize(int w, int h)
{
	// prevent division by zero when window is too short
	if (h == 0) { h = 1; }
	float ratio = w * 1.0 / h;

	// use projection matrix
	glMatrixMode(GL_PROJECTION);
	// reset matrix
	glLoadIdentity();

	// set the viewport to be entire window
	glViewport(0, 0, w, h);

	// set the correct perspective
	gluPerspective(45.0f, ratio, 0.1f, 100.0f);

	// get back to mode view
	glMatrixMode(GL_MODELVIEW);
}


void computePos(float deltaMoveWS, float deltaMoveAD)
{
	//cam_x += deltaMoveX * lx * 0.1f;
	//cam_z += deltaMoveZ * lz * 0.1f;
	cam_x += deltaMoveWS * sin(angle) * 0.1f;
	cam_x += deltaMoveAD * -1 * cos(angle) * 0.1f;
	cam_z += deltaMoveWS * -1 * cos(angle) * 0.1f;
	cam_z += deltaMoveAD * -1 * sin(angle) * 0.1f;

	ob_x = cam_x + lx;
	ob_z = cam_z + lz;

	std::cout << "cam_x: " << cam_x << std::endl;
	std::cout << "cam_z: " << cam_z << std::endl;
	std::cout << "ob_x: " << ob_x << std::endl;
	std::cout << "ob_z: " << ob_z << std::endl;
	std::cout << "lx: " << lx << std::endl;
	std::cout << "lz: " << lz << std::endl;
}


void computeDir(float deltaAngle)
{
	angle += deltaAngle;
	lx = sin(angle);
	lz = -cos(angle);

	ob_x = cam_x + lx;
	ob_z = cam_z + lz;

	std::cout << "cam_x: " << cam_x << std::endl;
	std::cout << "cam_z: " << cam_z << std::endl;
	std::cout << "ob_x: " << ob_x << std::endl;
	std::cout << "ob_z: " << ob_z << std::endl;
	std::cout << "lx: " << lx << std::endl;
	std::cout << "lz: " << lz << std::endl;
}


void renderScene()
{
	if (deltaMoveWS || deltaMoveAD) { 
		computePos(deltaMoveWS, deltaMoveAD);
	}
	if (deltaAngle) { 
		computeDir(deltaAngle);
	}

	// clear color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// reset transformation
	glLoadIdentity();

	// set camera
	gluLookAt(cam_x, 2.0f, cam_z, ob_x, 1.8f, ob_z, 0.0f, 1.0f, 0.0f);

	// draw grid
	drawGrid();

	// draw sphere
	drawSphere(&stPoints);

	glutSwapBuffers();

}


void pressSpecialKey(int key, int xx, int yy) {

	switch (key) {
	case GLUT_KEY_LEFT: deltaAngle = -0.01f; break;
	case GLUT_KEY_RIGHT: deltaAngle = 0.01f; break;
	case GLUT_KEY_UP: deltaMoveWS = 0.5f; break;
	case GLUT_KEY_DOWN: deltaMoveWS = -0.5f; break;
	}
}

void releaseSpecialKey(int key, int x, int y) {

	switch (key) {
	case GLUT_KEY_LEFT:
	case GLUT_KEY_RIGHT: deltaAngle = 0.0f; break;
	case GLUT_KEY_UP:
	case GLUT_KEY_DOWN: deltaMoveWS = 0; break;
	}
}

void drawAxes()
{
	// draw x axis
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glLineWidth(1);
	glVertex3f(-19, 0, 0);
	glVertex3f(19, 0, 0);
	glEnd();

	// draw y axis, which is visually z
	glBegin(GL_LINES);
	glColor3f(0, 1, 0);
	glLineWidth(1);
	glVertex3f(0, -19, 0);
	glVertex3f(0, 19, 0);
	glEnd();

	// draw z axis, which is visually y
	glBegin(GL_LINES);
	glColor3f(0, 0, 1);
	glLineWidth(1);
	glVertex3f(0, 0, -19);
	glVertex3f(0, 0, 19);
	glEnd();
}


void drawGrid()
{
	int i;
	for (i = 0; i < 40; i++)
	{
		glPushMatrix();
		if (i < 20) { glTranslatef(0 - 10, 0, i - 10); }
		if (i >= 20) { glTranslatef(i - 20 - 10, 0, 0 - 10); glRotatef(-90, 0, 1, 0); }

		glBegin(GL_LINES);
		glColor3f(0.5, 0.5, 0.5);
		glLineWidth(1);
		glVertex3f(0, -0, 0);
		glVertex3f(19, -0, 0);
		glEnd();
		glPopMatrix();
	}
}

void drawSphere(GUI::POINTS* pstPoints)
{
	int i = 0;
	for (i; i < pstPoints->dwNumOfPoints; i++)
	{
		glPushMatrix();
		glTranslatef(pstPoints->astPoint[i].pt_x, pstPoints->astPoint[i].pt_y, pstPoints->astPoint[i].pt_z);
		glColor3f(1.0f, 1.0f, 0);
		glutSolidSphere(0.01, 10, 10);
		glPopMatrix();
	}
}

void pressNormalKey(unsigned char key, int x, int y)
{
	if (key == 'r') { cam_x = 1.0f; cam_z = 5.0f; lx = 0.0f; lz = -1.0f; ob_x = cam_x + lx; ob_z = cam_z + lz; angle = 0.0f;  deltaAngle = 0.0f; deltaMoveWS = 0.0f; deltaMoveAD = 0.0f; }
	if (key == 'w') { deltaMoveWS = 0.5f; }
	if (key == 's') { deltaMoveWS = -0.5f; }
	if (key == 'a') { deltaMoveAD = 0.5f; }
	if (key == 'd') { deltaMoveAD = -0.5f; }
	glutPostRedisplay();
}


void releaseNormalKey(unsigned char key, int x, int y)
{
	if (key == 'w') { deltaMoveWS = 0; }
	if (key == 's') { deltaMoveWS = 0; }
	if (key == 'a') { deltaMoveAD = 0; }
	if (key == 'd') { deltaMoveAD = 0; }
	glutPostRedisplay();
}


void parsePoints(char* pchFileName, GUI::POINTS* pstPoints)
{
	std::ifstream ifsFile(pchFileName);
	std::string str, substr;
	int posComma;
	int counterPoint = 0;
	pstPoints->dwNumOfPoints = 0;

	while (!ifsFile.eof())
	{
		std::getline(ifsFile, str);

		// parse x
		posComma = str.find(',');
		substr = str.substr(0, posComma);
		pstPoints->astPoint[counterPoint].pt_x = std::stof(substr);

		// parse y
		str = str.substr(posComma + 1);
		posComma = str.find(',');
		substr = str.substr(0, posComma);
		pstPoints->astPoint[counterPoint].pt_z = std::stof(substr);

		// parse z
		str = str.substr(posComma + 1);
		pstPoints->astPoint[counterPoint].pt_y = std::stof(str);

		std::cout << pstPoints->astPoint[counterPoint].pt_x << ", ";
		std::cout << pstPoints->astPoint[counterPoint].pt_y << ", ";
		std::cout << pstPoints->astPoint[counterPoint].pt_z << std::endl;

		counterPoint += 1;
	}

	pstPoints->dwNumOfPoints = counterPoint;
}