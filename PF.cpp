/*---------------------------------------------------------*/
/* ----------------   PROYECTO FINAL  --------------------------*/
/*-----------------    2024-2   ---------------------------*/
/*------------- Alumno:                     ---------------*/
/*------------- No. Cuenta                  ---------------*/

#include <Windows.h>

#include <glad/glad.h>
#include <glfw3.h>						//main
#include <stdlib.h>		
#include <glm/glm.hpp>					//camera y model
#include <glm/gtc/matrix_transform.hpp>	//camera y model
#include <glm/gtc/type_ptr.hpp>
#include <time.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>					//Texture

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>

#include <shader_m.h>
#include <camera.h>
#include <modelAnim.h>
#include <model.h>
#include <Skybox.h>
#include <iostream>
#include <mmsystem.h>



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void my_input(GLFWwindow* window, int key, int scancode, int action, int mods);
void animate(void);

// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
GLFWmonitor* monitors;
GLuint VBO[3], VAO[3], EBO[3];

//Camera
Camera camera(glm::vec3(0.0f, 15.0f, 80.0f)); //PIDE LA POSICION INICIAL DENTRO DEL MUNDO VIRTUAL 
float MovementSpeed = 0.1f;
GLfloat lastX = SCR_WIDTH / 2.0f,
		lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

//Timing
const int FPS = 60;
const int LOOP_TIME = 1000 / FPS; // = 16 milisec // 1000 millisec == 1 sec
double	deltaTime = 0.0f,
lastFrame = 0.0f;

void getResolution(void);
void myData(void);							// De la practica 4
void LoadTextures(void);					// De la práctica 6
unsigned int generateTextures(char*, bool, bool);	// De la práctica 6

//For Keyboard
float	movX = 0.0f,
movY = 0.0f,
movZ = -5.0f,
rotX = 0.0f;

//Texture
unsigned int	
t_white,
t_pisoCentro,
t_ladrillos;


//Lighting
glm::vec3 lightPosition(0.0f, 4.0f, -10.0f);
glm::vec3 lightDirection(0.0f, -1.0f, -1.0f);

//// Light
glm::vec3 lightColor = glm::vec3(0.7f);
glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
glm::vec3 ambientColor = diffuseColor * glm::vec3(0.75f);

// posiciones
float	movAuto_x = 0.0f,
movAuto_z = 0.0f,
orienta = 90.0f;
bool	animacion = false,
recorrido1 = true,
recorrido2 = false,
recorrido3 = false,
recorrido4 = false;


//Keyframes (Manipulación y dibujo)
float	posX = 0.0f,
		posY = 0.0f,
		posZ = 0.0f,
		rotRodIzq = 0.0f,
		giroMonito = 0.0f;
float	incX = 0.0f,
		incY = 0.0f,
		incZ = 0.0f,
		rotRodIzqInc = 0.0f,
		giroMonitoInc = 0.0f;

#define MAX_FRAMES 9
int i_max_steps = 60;
int i_curr_steps = 0;
typedef struct _frame
{
	//Variables para GUARDAR Key Frames
	float posX;		//Variable para PosicionX
	float posY;		//Variable para PosicionY
	float posZ;		//Variable para PosicionZ
	float rotRodIzq;
	float giroMonito;

}FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 0;			//introducir número en caso de tener Key guardados
bool play = false;
int playIndex = 0;

void saveFrame(void)
{
	//printf("frameindex %d\n", FrameIndex);
	std::cout << "Frame Index = " << FrameIndex << std::endl;

	KeyFrame[FrameIndex].posX = posX;
	KeyFrame[FrameIndex].posY = posY;
	KeyFrame[FrameIndex].posZ = posZ;

	KeyFrame[FrameIndex].rotRodIzq = rotRodIzq;
	KeyFrame[FrameIndex].giroMonito = giroMonito;

	FrameIndex++;
}

void resetElements(void)
{
	posX = KeyFrame[0].posX;
	posY = KeyFrame[0].posY;
	posZ = KeyFrame[0].posZ;

	rotRodIzq = KeyFrame[0].rotRodIzq;
	giroMonito = KeyFrame[0].giroMonito;
}

void interpolation(void)
{
	incX = (KeyFrame[playIndex + 1].posX - KeyFrame[playIndex].posX) / i_max_steps;
	incY = (KeyFrame[playIndex + 1].posY - KeyFrame[playIndex].posY) / i_max_steps;
	incZ = (KeyFrame[playIndex + 1].posZ - KeyFrame[playIndex].posZ) / i_max_steps;

	rotRodIzqInc = (KeyFrame[playIndex + 1].rotRodIzq - KeyFrame[playIndex].rotRodIzq) / i_max_steps;
	giroMonitoInc = (KeyFrame[playIndex + 1].giroMonito - KeyFrame[playIndex].giroMonito) / i_max_steps;

}

unsigned int generateTextures(const char* filename, bool alfa, bool isPrimitive)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	
	if(isPrimitive)
		stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	else
		stbi_set_flip_vertically_on_load(false); // tell stb_image.h to flip loaded texture's on the y-axis.


	unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
	if (data)
	{
		if (alfa)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		return textureID;
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
		return 100;
	}

	stbi_image_free(data);
}

void LoadTextures()
{
	t_pisoCentro = generateTextures("Texturas/pisoCentro.jpg", 0, true);
	//This must be the last
	t_white = generateTextures("Texturas/white.jpg", 0, false);
}



void animate(void) 
{
	if (play)
	{
		if (i_curr_steps >= i_max_steps) //end of animation between frames?
		{
			playIndex++;
			if (playIndex > FrameIndex - 2)	//end of total animation?
			{
				std::cout << "Animation ended" << std::endl;
				//printf("termina anim\n");
				playIndex = 0;
				play = false;
			}
			else //Next frame interpolations
			{
				i_curr_steps = 0; //Reset counter
				//Interpolation
				interpolation();
			}
		}
		else
		{
			//Draw animation
			posX += incX;
			posY += incY;
			posZ += incZ;

			rotRodIzq += rotRodIzqInc;
			giroMonito += giroMonitoInc;

			i_curr_steps++;
		}
	}

	//Vehículo
	if (animacion)
	{
		movAuto_x += 3.0f;
	}
}

void getResolution() {
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	SCR_WIDTH = mode->width;
	SCR_HEIGHT = (mode->height) - 80;
}

void myData() {
	float vertices[] = {
		// positions          // texture coords
		 0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // top right
		 0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // bottom left
		-0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	float verticesPiso[] = {
		// positions          // texture coords AQUI MODIFICO EL PISO Y CUANTES VECES SE CLONA
		 10.5f,  10.5f, 0.0f,   16.0f, 16.0f, // top right
		 10.5f, -10.5f, 0.0f,   16.0f, 0.0f, // bottom right
		-10.5f, -10.5f, 0.0f,   0.0f, 0.0f, // bottom left
		-10.5f,  10.5f, 0.0f,   0.0f, 16.0f  // top left 
	};
	unsigned int indicesPiso[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	GLfloat verticesCubo[] = {
		//Position				//texture coords
		-0.5f, -0.5f, 0.5f,		0.0f, 0.0f,	//V0 - Frontal
		0.5f, -0.5f, 0.5f,		1.0f, 0.0f,	//V1
		0.5f, 0.5f, 0.5f,		1.0f, 1.0f,	//V5
		-0.5f, -0.5f, 0.5f,		0.0f, 0.0f,	//V0
		-0.5f, 0.5f, 0.5f,		0.0f, 1.0f,	//V4
		0.5f, 0.5f, 0.5f,		1.0f, 1.0f,	//V5

		0.5f, -0.5f, -0.5f,		0.0f, 0.0f,	//V2 - Trasera
		-0.5f, -0.5f, -0.5f,	1.0f, 0.0f,	//V3
		-0.5f, 0.5f, -0.5f,		1.0f, 1.0f,	//V7
		0.5f, -0.5f, -0.5f,		0.0f, 0.0f,	//V2
		0.5f, 0.5f, -0.5f,		0.0f, 1.0f,	//V6
		-0.5f, 0.5f, -0.5f,		1.0f, 1.0f,	//V7

		-0.5f, 0.5f, 0.5f,		0.0f, 1.0f,	//V4 - Izq
		-0.5f, 0.5f, -0.5f,		0.0f, 1.0f,	//V7
		-0.5f, -0.5f, -0.5f,	0.0f, 1.0f,	//V3
		-0.5f, -0.5f, -0.5f,	0.0f, 1.0f,	//V3
		-0.5f, 0.5f, 0.5f,		0.0f, 1.0f,	//V4
		-0.5f, -0.5f, 0.5f,		0.0f, 1.0f,	//V0

		0.5f, 0.5f, 0.5f,		1.0f, 0.0f,	//V5 - Der
		0.5f, -0.5f, 0.5f,		1.0f, 0.0f,	//V1
		0.5f, -0.5f, -0.5f,		1.0f, 0.0f,	//V2
		0.5f, 0.5f, 0.5f,		1.0f, 0.0f,	//V5
		0.5f, 0.5f, -0.5f,		1.0f, 0.0f,	//V6
		0.5f, -0.5f, -0.5f,		1.0f, 0.0f,	//V2

		-0.5f, 0.5f, 0.5f,		0.0f, 1.0f,	//V4 - Sup
		0.5f, 0.5f, 0.5f,		0.0f, 1.0f,	//V5
		0.5f, 0.5f, -0.5f,		0.0f, 1.0f,	//V6
		-0.5f, 0.5f, 0.5f,		0.0f, 1.0f,	//V4
		-0.5f, 0.5f, -0.5f,		0.0f, 1.0f,	//V7
		0.5f, 0.5f, -0.5f,		0.0f, 1.0f,	//V6

		-0.5f, -0.5f, 0.5f,		1.0f, 1.0f,	//V0 - Inf
		-0.5f, -0.5f, -0.5f,	1.0f, 1.0f,	//V3
		0.5f, -0.5f, -0.5f,		1.0f, 1.0f,	//V2
		-0.5f, -0.5f, 0.5f,		1.0f, 1.0f,	//V0
		0.5f, -0.5f, -0.5f,		1.0f, 1.0f,	//V2
		0.5f, -0.5f, 0.5f,		1.0f, 1.0f,	//V1
	};

	glGenVertexArrays(3, VAO);
	glGenBuffers(3, VBO);
	glGenBuffers(3, EBO);

	glBindVertexArray(VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//Para Piso
	glBindVertexArray(VAO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesPiso), verticesPiso, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesPiso), indicesPiso, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

int main() {
	// glfw: initialize and configure
	glfwInit();

	// glfw window creation
	monitors = glfwGetPrimaryMonitor();
	getResolution();

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Pratica 6 2024-2", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetWindowPos(window, 0, 30);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, my_input);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	//Mis funciones
	//Datos a utilizar
	LoadTextures();
	myData();
	glEnable(GL_DEPTH_TEST);

	

	// build and compile shaders
	// -------------------------
	Shader myShader("shaders/shader_texture_color.vs", "shaders/shader_texture_color.fs"); //To use with primitives
	Shader staticShader("Shaders/shader_Lights.vs", "Shaders/shader_Lights_mod.fs");	//To use with static models
	Shader skyboxShader("Shaders/skybox.vs", "Shaders/skybox.fs");	//To use with skybox
	Shader animShader("Shaders/anim.vs", "Shaders/anim.fs");	//To use with animated models 
	
	vector<std::string> faces{
		"resources/skybox/right.jpg",
		"resources/skybox/left.jpg",
		"resources/skybox/top.jpg",
		"resources/skybox/bottom.jpg",
		"resources/skybox/front.jpg",
		"resources/skybox/back.jpg"
	};

	Skybox skybox = Skybox(faces);

	// Shader configuration
	// --------------------
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	// load models
	// -----------
	Model piso("resources/objects/piso/piso.obj");
	//**********************  DISEÑO PROYECTO  *****************************//

	Model MuroA("resources/objects/MuroA/muroA.obj");
	Model MuroB("resources/objects/MuroB/MuroB.obj");
	Model MuroC("resources/objects/MuroC/MuroC.obj");
	Model SepLoc("resources/objects/SepLoc/SepLoc.obj");
	Model Fachadas("resources/objects/Fachadas/Fachadas.obj");
	Model ZVCE("resources/objects/ZonaVCE/ZonaVCE.obj");
	Model Macetas("resources/objects/MacetasPasillo/MacetasPas.obj");
	Model BancasPas("resources/objects/BancaPas/BancasPas.obj");
	Model Stand1("resources/objects/Stand1/StandChips.obj");
	Model Stand2("resources/objects/Stand2/StandCaramels.obj");
	Model Stand3("resources/objects/Stand3/StandSodas.obj");
	Model Stand4("resources/objects/Stand4/StandHot.obj");
	Model Fuente("resources/objects/Fuente/Fuente.obj");
	Model Inflable("resources/objects/Inflable/Inflable.obj");
	Model Brincolin("resources/objects/Brincolin/Brincolin.obj");
	Model Carro1("resources/objects/Carro1/Carro1.obj");
	Model Carro2("resources/objects/Carro2/Carro2.obj");
	Model Carro3("resources/objects/Carro3/Carro3.obj");
	Model Carro4("resources/objects/Carro4/Carro4.obj");
	Model Carro5("resources/objects/Carro5/Carro5.obj");
	Model Carro6("resources/objects/Carro6/Carro6.obj");
	Model Carro7("resources/objects/Carro7/Carro7.obj");
	Model Carro8("resources/objects/Carro8/Carro8.obj");
	Model Carro9("resources/objects/Carro9/Carro9.obj");
	Model Carro10("resources/objects/Carro10/Carro10.obj");
	Model MesasZC("resources/objects/MesasZC/MesasZC.obj");
	Model SillasZC("resources/objects/SillasZC/SillasZC.obj");
	Model PisoPasto("resources/objects/PastoZV/PastoZV.obj");
	Model BancasZV("resources/objects/BancasZV/BancasZV.obj");
	Model Pinos("resources/objects/Pinos/Pinos.obj");
	Model Arboles("resources/objects/Arbol/Arbol.obj");
	Model Palmeras("resources/objects/Palmeras/Palmeras.obj");
	Model Piedras("resources/objects/Piedras/Piedras.obj");
	Model Piedra("resources/objects/Piedra/Piedra.obj");
	Model Carrito1("resources/objects/Carrito1/Carrito1.obj");
	Model Carrito2("resources/objects/Carrito2/Carrito2.obj");
	Model Exibidor1("resources/objects/Exibidor1/Exibidor1.obj");
	Model Exibidor2("resources/objects/Exibidor2/Exibidor2.obj");
	Model Mostrador("resources/objects/Mostrador/Mostrador.obj");
	Model CajaPasillo("resources/objects/CajaPasillo/CajaPasillo.obj");
	Model CajaAutoCobro("resources/objects/CajaAutoCobro/CajaAutoCobro.obj");
	Model CajaSalida("resources/objects/CajaSalida/CajaSalida.obj");
	Model Est1Wal("resources/objects/Est1Wal/Est1Wal.obj");
	Model Est2Wal("resources/objects/Est2Wal/Est2Wal.obj");
	Model Est3Wal("resources/objects/Est3Wal/Est3Wal.obj");
	Model Est4Wal("resources/objects/Est4Wal/Est4Wal.obj");
	Model Est5Wal("resources/objects/Est5Wal/Est5Wal.obj");
	Model Est6Wal("resources/objects/Est6Wal/Est6Wal.obj");
	Model Est7Wal("resources/objects/Est7Wal/Est7Wal.obj");
	Model Est1Ber("resources/objects/Est1Ber/Est1Ber.obj");
	Model Est2Ber("resources/objects/Est2Ber/Est2Ber.obj");
	Model Ropa1("resources/objects/Ropa1/Ropa1.obj");
	Model Ropa2("resources/objects/Ropa2/Ropa2.obj");
	Model Ropa3("resources/objects/Ropa3/Ropa3.obj");
	Model Maniqui1("resources/objects/Maniqui1/Maniqui1.obj");
	Model Maniqui2("resources/objects/Maniqui2/Maniqui2.obj");
	Model Maniqui3("resources/objects/Maniqui3/Maniqui3.obj");
	Model Maniqui4("resources/objects/Maniqui4/Maniqui4.obj");
	Model Espejos("resources/objects/Espejos/Espejos.obj");
	Model Est1Min("resources/objects/Est1Min/Est1Min.obj");
	Model Est2Min("resources/objects/Est2Min/Est2Min.obj");
	Model Est3Min("resources/objects/Est3Min/Est3Min.obj");
	Model Est4Min("resources/objects/Est4Min/Est4Min.obj");
	Model Est5Min("resources/objects/Est5Min/Est5Min.obj");
	Model ATM("resources/objects/ATM/ATM.obj");
	Model MesaBanco("resources/objects/MesaBanco/MesaBanco.obj");
	Model SillaBanco("resources/objects/SillaBanco/SillaBanco.obj");
	Model EscritorioBanco("resources/objects/EscritorioBanco/EscritorioBanco.obj");
	Model Wc("resources/objects/Wc/wc.obj");
	Model PuertasWC("resources/objects/Wc/PuertasWC.obj");
	Model Lavabo("resources/objects/Lavabo/Lavabo.obj");
	Model Carriolas("resources/objects/Carriolas/Carriolas.obj");
	Model Bebe("resources/objects/Bebe/Bebe.obj");
	Model BebeFeo("resources/objects/BebeFeo/BebeFeo.obj");
	Model Abuela1("resources/objects/Abuela1/Abuela1.obj");
	Model Abuela2("resources/objects/Abuela2/Abuela2.obj");
	Model Abuelo1("resources/objects/Abuelo1/Abuelo1.obj");
	Model Abuelo2("resources/objects/Abuelo2/Abuelo2.obj");
	Model Nina1("resources/objects/Nina1/Nina1.obj");
	Model Nina2("resources/objects/Nina2/Nina2.obj");
	Model Nina3("resources/objects/Nina3/Nina3.obj");
	Model Nino1("resources/objects/Nino1/Nino1.obj");
	Model Nino2("resources/objects/Nino2/Nino2.obj");
	Model Nino3("resources/objects/Nino3/Nino3.obj");
	Model H1("resources/objects/H1/H1.obj");
	Model H2("resources/objects/H2/H2.obj");
	Model H3("resources/objects/H3/H3.obj");
	Model H4("resources/objects/H4/H4.obj");
	Model M1("resources/objects/M1/M1.obj");
	Model M2("resources/objects/M2/M2.obj");
	Model M3("resources/objects/M3/M3.obj");
	Model M4("resources/objects/M4/M4.obj");
	Model M5("resources/objects/M5/M5.obj");
	//**************** MODELOS PARA LA ANIMACIONES COMPLEJAS ************************
	ModelAnim animacionPersonaje("resources/objects/Personaje1/Arm.dae");
	animacionPersonaje.initShaders(animShader.ID);
	Model cabezaBuzz("resources/objects/Buzz/buzzlightyHead.obj");
	Model hipBuzz("resources/objects/Buzz/buzzlightyHip.obj");
	Model BrazoIzqBuzz("resources/objects/Buzz/buzzlightyLeftArm.obj");
	
	
	
	
	
	//*****************************************************************************

	//Inicialización de KeyFrames
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		KeyFrame[i].posX = 0;
		KeyFrame[i].posY = 0;
		KeyFrame[i].posZ = 0;
		KeyFrame[i].rotRodIzq = 0;
		KeyFrame[i].giroMonito = 0;
	}


	// create transformations and Projection
	glm::mat4 modelOp = glm::mat4(1.0f);		// initialize Matrix, Use this matrix for individual models
	glm::mat4 viewOp = glm::mat4(1.0f);		//Use this matrix for ALL models
	glm::mat4 projectionOp = glm::mat4(1.0f);	//This matrix is for Projection

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		skyboxShader.setInt("skybox", 0);

		// per-frame time logic
		// --------------------
		lastFrame = SDL_GetTicks();

		// input
		// -----
		//my_input(window);
		animate();

		// render
		// ------
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		//Setup shader for static models
		staticShader.use();
		//Setup Advanced Lights
		staticShader.setVec3("viewPos", camera.Position);
		staticShader.setVec3("dirLight.direction", lightDirection);
		staticShader.setVec3("dirLight.ambient", ambientColor);
		staticShader.setVec3("dirLight.diffuse", diffuseColor);
		staticShader.setVec3("dirLight.specular", glm::vec3(0.6f, 0.6f, 0.6f));

		staticShader.setVec3("pointLight[0].position", lightPosition);
		staticShader.setVec3("pointLight[0].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[0].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[0].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("pointLight[0].constant", 0.08f);
		staticShader.setFloat("pointLight[0].linear", 0.009f);
		staticShader.setFloat("pointLight[0].quadratic", 0.032f);

		staticShader.setVec3("pointLight[1].position", glm::vec3(-80.0, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[1].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[1].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[1].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("pointLight[1].constant", 1.0f);
		staticShader.setFloat("pointLight[1].linear", 0.009f);
		staticShader.setFloat("pointLight[1].quadratic", 0.032f);

		staticShader.setVec3("spotLight[0].position", glm::vec3(0.0f, 20.0f, 10.0f));
		staticShader.setVec3("spotLight[0].direction", glm::vec3(0.0f, -1.0f, 0.0f));
		staticShader.setVec3("spotLight[0].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("spotLight[0].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("spotLight[0].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("spotLight[0].cutOff", glm::cos(glm::radians(10.0f)));
		staticShader.setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(60.0f)));
		staticShader.setFloat("spotLight[0].constant", 1.0f);
		staticShader.setFloat("spotLight[0].linear", 0.0009f);
		staticShader.setFloat("spotLight[0].quadratic", 0.0005f);

		staticShader.setFloat("material_shininess", 32.0f);

		//glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 tmp = glm::mat4(1.0f);
		// view/projection transformations
		//glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		projectionOp = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		viewOp = camera.GetViewMatrix();
		staticShader.setMat4("projection", projectionOp);
		staticShader.setMat4("view", viewOp);

		//Setup shader for primitives
		myShader.use();
		// view/projection transformations
		//projectionOp = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 400.0f);
		viewOp = camera.GetViewMatrix();
		// pass them to the shaders
		//myShader.setMat4("model", modelOp);
		myShader.setMat4("view", viewOp);
		// note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
		myShader.setMat4("projection", projectionOp);
		/**********/


		// -------------------------------------------------------------------------------------------------------------------------
		// Personaje Animacion
		// -------------------------------------------------------------------------------------------------------------------------
		//Remember to activate the shader with the animation
		animShader.use();
		animShader.setMat4("projection", projectionOp);
		animShader.setMat4("view", viewOp);

		animShader.setVec3("material.specular", glm::vec3(0.5f));
		animShader.setFloat("material.shininess", 32.0f);
		animShader.setVec3("light.ambient", ambientColor);
		animShader.setVec3("light.diffuse", diffuseColor);
		animShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		animShader.setVec3("light.direction", lightDirection);
		animShader.setVec3("viewPos", camera.Position);

		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(-40.3f, 1.75f, 0.3f)); // translate it down so it's at the center of the scene
		modelOp = glm::scale(modelOp, glm::vec3(0.00001f));	// it's a bit too big for our scene, so scale it down
		modelOp = glm::rotate(modelOp, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		animShader.setMat4("model", modelOp);
		animacionPersonaje.Draw(animShader);
		// -------------------------------------------------------------------------------------------------------------------------
		// Segundo Personaje Animacion
		// -------------------------------------------------------------------------------------------------------------------------




		// -------------------------------------------------------------------------------------------------------------------------
		// Escenario Primitivas
		// -------------------------------------------------------------------------------------------------------------------------
		myShader.use();

		//Tener Piso como referencia
		glBindVertexArray(VAO[2]);
		//Colocar código aquí
		modelOp = glm::scale(glm::mat4(1.0f), glm::vec3(40.0f, 2.0f, 40.0f));
		modelOp = glm::translate(modelOp, glm::vec3(0.0f, -1.0f, 0.0f));
		modelOp = glm::rotate(modelOp, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		myShader.setMat4("model", modelOp);
		myShader.setVec3("aColor", 1.0f, 1.0f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, t_pisoCentro);//DIBUJO Y CAMBIO DE PISO 
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindVertexArray(VAO[0]);
		//Colocar código aquí
		/*modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, 0.0f));
		modelOp = glm::scale(modelOp, glm::vec3(5.0f, 5.0f, 1.0f));
		myShader.setMat4("model", modelOp);
		myShader.setVec3("aColor", 1.0f, 1.0f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, t_unam);*/
		//glDrawArrays(GL_TRIANGLES, 0, 36); //A lonely cube :(
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		/***   Segundo objeto  **/
		/*
		glBindVertexArray(VAO[1]);
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
		myShader.setMat4("model", modelOp);
		myShader.setVec3("aColor", 1.0f, 1.0f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, t_unam);
		glDrawArrays(GL_TRIANGLES, 0, 36); //A lonely cube :(
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		*/
		glBindVertexArray(0);
		// ------------------------------------------------------------------------------------------------------------------------
		// Termina Escenario Primitivas
		// -------------------------------------------------------------------------------------------------------------------------

		// -------------------------------------------------------------------------------------------------------------------------
		// Escenario
		// -------------------------------------------------------------------------------------------------------------------------
		staticShader.use();
		staticShader.setMat4("projection", projectionOp);
		staticShader.setMat4("view", viewOp);

		// ********* DIBUJO DEL CENTRO COMERCIAL*************
		// MUROS - MuroA parte izq vista superio
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));//A todo .obj agregar esta translacion  
		staticShader.setMat4("model", modelOp);
		MuroA.Draw(staticShader);
		//MuroB parte trasera vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));  
		staticShader.setMat4("model", modelOp);
		MuroB.Draw(staticShader);
		//MuroC parte der vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));  
		staticShader.setMat4("model", modelOp);
		MuroC.Draw(staticShader);
		//Separacion Locales vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));  
		staticShader.setMat4("model", modelOp);
		SepLoc.Draw(staticShader);
		//Fachada Locales vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Fachadas.Draw(staticShader);
		//Fachada Zona Verde, Comida y Estacionamiento vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		ZVCE.Draw(staticShader);
		//Macetas Pasillo Principal vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Macetas.Draw(staticShader);
		//Bancas Pasillo Principal vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		BancasPas.Draw(staticShader);
		//StandChips Pasillo Principal vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Stand1.Draw(staticShader);
		//StandCaramels Pasillo Principal vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Stand2.Draw(staticShader);
		//StandSoda Pasillo Principal vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Stand3.Draw(staticShader);
		//StandHotDog Pasillo Principal vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Stand4.Draw(staticShader);
		//Fuente Zona Verde vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Fuente.Draw(staticShader);
		//Inflable Zona Verde vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Inflable.Draw(staticShader);
		//Brincolin Zona Verde vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Brincolin.Draw(staticShader);
		//Carro Rojo 1  vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Carro1.Draw(staticShader);
		//Carro Negro 1 vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Carro2.Draw(staticShader);
		//Carro Rojo 2 vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Carro3.Draw(staticShader);
		//Carro Negro 2 vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Carro4.Draw(staticShader);
		//Carro Verde 1 vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Carro5.Draw(staticShader);
		//Carro Verde 2 vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Carro6.Draw(staticShader);
		//Carro Amarillo 1 vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Carro7.Draw(staticShader);
		//Carro Amarillo 2 vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Carro8.Draw(staticShader);
		//Carro Azul 1 vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Carro9.Draw(staticShader);
		//Carro Azul 2 vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Carro10.Draw(staticShader);
		//Mesas Zona Comida vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		MesasZC.Draw(staticShader);
		//Sillas Zona Comida vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		SillasZC.Draw(staticShader);
		//Piso Zona Verde vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		PisoPasto.Draw(staticShader);
		//Bancas Zona Verde vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		BancasZV.Draw(staticShader);
		//Pinos Zona Verde vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Pinos.Draw(staticShader);
		//Arboles Zona Verde vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Arboles.Draw(staticShader);
		//Arboles Zona Verde vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Palmeras.Draw(staticShader);
		//Piedras Zona Verde vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Piedras.Draw(staticShader);
		//Piedra Zona Verde vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Piedra.Draw(staticShader);
		//Carrito 1 Walmart  vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Carrito1.Draw(staticShader);
		//Carrito 2 Walmart vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Carrito2.Draw(staticShader);
		//Exhibidor 1 Walmart vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Exibidor1.Draw(staticShader);
		//Exhibidor 2 Walmart vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Exibidor2.Draw(staticShader);
		//Caja Mostradores Walmart - Bershka y Miniso vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Mostrador.Draw(staticShader);
		//Caja Pasillo Walmart vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		CajaPasillo.Draw(staticShader);
		//Caja Pasillo Walmart vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		CajaAutoCobro.Draw(staticShader);
		//Caja Salida Walmart vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		CajaSalida.Draw(staticShader);
		//Estante 1 Walmart vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Est1Wal.Draw(staticShader);
		//Estante 2 Walmart vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Est2Wal.Draw(staticShader);
		//Estante 3 Walmart vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Est3Wal.Draw(staticShader);
		//Estante 4 Walmart vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Est4Wal.Draw(staticShader);
		//Estante 5 Walmart vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Est5Wal.Draw(staticShader);
		//Estante 6 Walmart vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Est6Wal.Draw(staticShader);
		//Estante 7 Walmart vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Est7Wal.Draw(staticShader);
		//Estante 1 Bershka vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Est1Ber.Draw(staticShader);
		//Estante 2 Bershka vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Est2Ber.Draw(staticShader);
		//Ropa 1 Bershka vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Ropa1.Draw(staticShader);
		//Ropa 2 Bershka vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Ropa2.Draw(staticShader);
		//Ropa 3 Bershka vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Ropa3.Draw(staticShader);
		//Maniqui 1 Bershka vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Maniqui1.Draw(staticShader);
		//Maniqui 2 Bershka vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Maniqui2.Draw(staticShader);
		//Maniqui 3 Bershka vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Maniqui3.Draw(staticShader);
		//Maniqui 4 Bershka vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Maniqui4.Draw(staticShader);
		//Espejos Bershka vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Espejos.Draw(staticShader);
		//Estante 1 Miniso vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Est1Min.Draw(staticShader);
		//Estante 2 Miniso vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Est2Min.Draw(staticShader);
		//Estante 3 Miniso vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Est3Min.Draw(staticShader);
		//Estante 4 Miniso vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Est4Min.Draw(staticShader);
		//Estante 5 Miniso vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Est5Min.Draw(staticShader);
		//ATM Banco vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		ATM.Draw(staticShader);
		//Mesas Banco vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		MesaBanco.Draw(staticShader);
		//Sillas Banco vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		SillaBanco.Draw(staticShader);
		//Escritorio Banco vista superior
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		EscritorioBanco.Draw(staticShader);
		//Tazas de baño WC
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));  
		staticShader.setMat4("model", modelOp);
		Wc.Draw(staticShader);
		//Puertas WC
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));  
		staticShader.setMat4("model", modelOp);
		PuertasWC.Draw(staticShader);
		//Lavabos WC
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Lavabo.Draw(staticShader);
		//Carriolas 
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Carriolas.Draw(staticShader);
		//Bebe Pelon 
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Bebe.Draw(staticShader);
		//Bebe Pelon en Andadera
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		BebeFeo.Draw(staticShader);
		//Abuela 1
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Abuela1.Draw(staticShader);
		//Abuela 2
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Abuela2.Draw(staticShader);
		//Abuelo 1
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Abuelo1.Draw(staticShader);
		//Abuelo 2
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Abuelo2.Draw(staticShader);
		//Niña 1
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Nina1.Draw(staticShader);
		//Niña 2
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Nina2.Draw(staticShader);
		//Niña 3
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Nina3.Draw(staticShader);
		//Niño 1
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Nino1.Draw(staticShader);
		//Niño 2
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Nino2.Draw(staticShader);
		//Niño 3
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		Nino3.Draw(staticShader);
		//Hombre 1
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		H1.Draw(staticShader);
		//Hombre 2
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		H2.Draw(staticShader);
		//Hombre 3
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		H3.Draw(staticShader);
		//Hombre 4
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		H4.Draw(staticShader);
		//Mujer 1
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		M1.Draw(staticShader);
		//Mujer 2
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		M2.Draw(staticShader);
		//Mujer 3
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		M3.Draw(staticShader);
		//Mujer 4
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		M4.Draw(staticShader);
		//Mujer 5
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		M5.Draw(staticShader);
		//TERMINE DE IMPORTAR LOS OBJETOS ESTATICOS Y EL MODELO EN GENERAL DE LA PLAZA COMERCIAL

		//COMIENZA LA IMPORTACION DE LOS MODELOS COMPLEJOS
		Model cabezaBuzz("resources/objects/Buzz/buzzlightyHead.obj");
		Model hipBuzz("resources/objects/Buzz/buzzlightyHip.obj");
		Model BrazoIzqBuzz("resources/objects/Buzz/buzzlightyLeftArm.obj");
		//BuzzLightYear cabeza
		//modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.9f, 0.0f));
		staticShader.setMat4("model", modelOp);
		cabezaBuzz.Draw(staticShader);
		//BuzzLightYear 
		staticShader.setMat4("model", modelOp);
		hipBuzz.Draw(staticShader);
		//BuzzLightYear 
		staticShader.setMat4("model", modelOp);
		BrazoIzqBuzz.Draw(staticShader);

		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.75f, 0.0f));
		modelOp = glm::scale(modelOp, glm::vec3(0.2f));
		staticShader.setMat4("model", modelOp);
		//piso.Draw(staticShader);


		// -------------------------------------------------------------------------------------------------------------------------
		// Carro
		// -------------------------------------------------------------------------------------------------------------------------
		
		// -------------------------------------------------------------------------------------------------------------------------
		// Personaje
		// -------------------------------------------------------------------------------------------------------------------------
		
		
		
		
		// -------------------------------------------------------------------------------------------------------------------------
		// Just in case
		// -------------------------------------------------------------------------------------------------------------------------
		
		//-------------------------------------------------------------------------------------
		// draw skybox as last
		// -------------------
		skyboxShader.use();
		skybox.Draw(skyboxShader, viewOp, projectionOp, camera);

		// Limitar el framerate a 60
		deltaTime = SDL_GetTicks() - lastFrame; // time for full 1 loop
		if (deltaTime < LOOP_TIME)
		{
			SDL_Delay((int)(LOOP_TIME - deltaTime));
		}

		
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glDeleteVertexArrays(2, VAO);
	glDeleteBuffers(2, VBO);
	//skybox.Terminate();
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void my_input(GLFWwindow* window, int key, int scancode, int action, int mode) 
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, (float)deltaTime);

	//To Configure Model
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
		posZ++;
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
		posZ--;
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		posX--;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		posX++;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		rotRodIzq--;
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		rotRodIzq++;
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
		giroMonito--;
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		giroMonito++;
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		lightPosition.x++;
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		lightPosition.x--;

	//Car animation
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		animacion ^= true;

	//To play KeyFrame animation 
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		if (play == false && (FrameIndex > 1))
		{
			std::cout << "Play animation" << std::endl;
			resetElements();
			//First Interpolation				
			interpolation();

			play = true;
			playIndex = 0;
			i_curr_steps = 0;
		}
		else
		{
			play = false;
			std::cout << "Not enough Key Frames" << std::endl;
		}
	}

	//To Save a KeyFrame
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		if (FrameIndex < MAX_FRAMES)
		{
			saveFrame();
		}
	}

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos) 
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
}