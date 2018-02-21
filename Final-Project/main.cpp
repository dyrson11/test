#include <bits/stdc++.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "include/model.h"
#include "include/graph.h"
#include "include/program.h"
#include "include/variables.h"
#include "include/flowline.h"

#include "andres/graph/complete-graph.hxx"
#include "andres/graph/multicut/greedy-additive.hxx"
#include "graph/include/andres/graph/lifting.hxx"
#include "graph/src/command-line-tools/utils.hxx"

template<typename N, typename E>
void readResult(model<N,E> mod)
{

}

using namespace glm;

void reshapefunc(int width,int height)
{
	winwidth=width;
	winheight=height;

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
    glViewport(0,0,width,height);
}

void init (void)
{
	freopen("log.c", "w", stderr);

	obj.load_model("res/models/mug/mug.obj");
    //obj.load_model("res/models/wineglass/wineglass.obj");
	//obj.load_model("res/models/atangana.obj");
	cout<<"readed obj\n";
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
	glGenBuffers(1, &IBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * obj.positions.size(), obj.positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
	tGraph.init_graph( obj );

	cout<<obj.indices.size()<<endl;
	cout<<"Starting Correlation clustering...\n";
	vector<char> edge_labels( tGraph.sampleGraph.numberOfEdges(), 1 );
	andres::graph::multicut::greedyAdditiveEdgeContraction( tGraph.sampleGraph, tGraph.weights, edge_labels );

	auto energy_value = inner_product(tGraph.weights.begin(), tGraph.weights.end(), edge_labels.begin(), .0);

    std::vector<size_t> vertex_labels( tGraph.sampleGraph.numberOfVertices() );
    edgeToVertexLabels( tGraph.sampleGraph, edge_labels, vertex_labels );

	cout<<"Done.\n";
	nClusters = *max_element(vertex_labels.begin(), vertex_labels.end()) + 1;
	cout << "Number of clusters: " << nClusters << endl;

	for(int i = 0; i < vertex_labels.size(); i++)
	{
		model<float, float>::Line* tLine = obj.lines[i];
		tLine->clusterID = vertex_labels[i];
	}

	colors.push_back( vec3( 0.5, 0.0, 0.0 ) );
	colors.push_back( vec3( 0.0, 0.5, 0.0 ) );
	colors.push_back( vec3( 0.0, 0.0, 0.5 ) );

	vector<FlowLine> initialFlowLines;
	bool searching = true;
	for( auto& tLine : obj.lines )
	{
		if( tLine->isInFlowline )
			continue;
		FlowLine tFlow;
		tFlow.lines.push_back( tLine->index );
		tLine->isInFlowline = true;
		tLine->flowLineID = initialFlowLines.size();
		auto node = tLine;
		while(searching)
		{
			searching = false;
			for( auto& neighbor : node->vertices[0]->lines )
			{
				if( neighbor->clusterID != tLine->clusterID || neighbor->isInFlowline )
					continue;

				tFlow.lines.push_back( neighbor->index );
				neighbor->isInFlowline = true;
				neighbor->flowLineID = initialFlowLines.size();
				node = neighbor;
				searching = true;
				break;
			}
		}
		searching = true;
		node = tLine;
		while(searching)
		{
			searching = false;
			for( auto& neighbor : node->vertices[1]->lines )
			{
				if( neighbor->clusterID != tLine->clusterID || neighbor->isInFlowline )
					continue;

				tFlow.lines.push_back( neighbor->index );
				neighbor->isInFlowline = true;
				neighbor->flowLineID = initialFlowLines.size();
				node = neighbor;
				searching = true;
				break;
			}
		}
		initialFlowLines.push_back( tFlow );
	}

	nFlowLines = initialFlowLines.size();
	cout << "Number of FlowLines: " << nFlowLines << endl;

	obj.updateModelFlowlines();

	glBindBuffer(GL_ARRAY_BUFFER, IBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint) * obj.ids.size(), obj.ids.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 1 * sizeof(GLuint), (void*)0);
    glEnableVertexAttribArray(1);



    program1.loadShaders("res/shaders/vertexShader.glsl", "res/shaders/fragmentShader.glsl");
	glUseProgram(program1.id);
}

void display (void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program1.id);
	view = lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glUniformMatrix4fv(glGetUniformLocation(program1.id, "view"), 1, GL_FALSE, &view[0][0]);
	mat4 projection = perspective(radians(45.0f), (float)winwidth / (float)winheight, 0.1f, 100.0f);
	glUniformMatrix4fv(glGetUniformLocation(program1.id, "projection"), 1, GL_FALSE, &projection[0][0]);

	glLineWidth(3.0);
	glDrawElements( GL_LINES, obj.indices.size(), GL_UNSIGNED_INT, obj.indices.data());

	mat4 model;
	model = scale(model, vec3(0.25f, 0.25f, 0.25f));
	glUniformMatrix4fv(glGetUniformLocation(program1.id, "model"), 1, GL_FALSE, value_ptr(model));



	//glDrawElements( GL_LINES, sizeof(vec2) * obj.indices.size(), GL_UNSIGNED_INT, nullptr);
    glutSwapBuffers();
}

void motionfunc(int x,int y)
{
    if (firstMouse)
    {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }

    float xoffset = x - lastX;
    float yoffset = lastY - y; // reversed since y-coordinates go from bottom to top
    lastX = x;
    lastY = y;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw1 += xoffset;
    pitch1 += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch1 > 89.0f)
        pitch1 = 89.0f;
    if (pitch1 < -89.0f)
        pitch1 = -89.0f;


    vec3 front;
    front.x = cos(radians(yaw1)) * cos(radians(pitch1));
    front.y = sin(radians(pitch1));
    front.z = sin(radians(yaw1)) * cos(radians(pitch1));
    cameraFront = normalize(front);
}

void keyboardfunc(unsigned char key,int x,int y)
{
	if (key=='q' || key==27) exit(0);
    float cameraSpeed = 2.5f * deltaTime;
    switch (key)
    {
        case 'w': case 'W':
            cameraPos.x = cameraPos.x + 0.02 * cameraFront.x;
			cameraPos.y = cameraPos.y + 0.02 * cameraFront.y;
			cameraPos.z = cameraPos.z + 0.02 * cameraFront.z;
            break;
        case 'a': case 'A':
			cameraPos.x = cameraPos.x - 0.02 * normalize(cross(cameraFront, cameraUp)).x;
			cameraPos.y = cameraPos.y - 0.02 * normalize(cross(cameraFront, cameraUp)).y;
			cameraPos.z = cameraPos.z - 0.02 * normalize(cross(cameraFront, cameraUp)).z;
            //cameraPos -= normalize(cross(cameraFront, cameraUp)) * 0.2;
            break;
        case 's': case 'S':
			cameraPos.x = cameraPos.x - 0.02 * cameraFront.x;
			cameraPos.y = cameraPos.y - 0.02 * cameraFront.y;
			cameraPos.z = cameraPos.z - 0.02 * cameraFront.z;
            break;
        case 'd': case 'D':
			cameraPos.x = cameraPos.x + 0.02 * normalize(cross(cameraFront, cameraUp)).x;
			cameraPos.y = cameraPos.y + 0.02 * normalize(cross(cameraFront, cameraUp)).y;
			cameraPos.z = cameraPos.z + 0.02 * normalize(cross(cameraFront, cameraUp)).z;
            //cameraPos += normalize(cross(cameraFront, cameraUp)) * 0.2;
            break;
    }
    //std::cout<<cameraPos<<std::endl;
    //std::cout<<0.05 * cameraFront<<std::endl;
    glutPostRedisplay();
}

int main (int argc,char** argv)
{
    winwidth=winheight=512;

    glutInit(&argc,argv);
    glutInitWindowSize(winwidth,winheight);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
	glutCreateWindow("FlowRep");

    if ( glewInit() != GLEW_OK )
    {
        return -1;
    }

    init();
	glutDisplayFunc(display);
    glutReshapeFunc(reshapefunc);
    glutIdleFunc(display);
	glutKeyboardFunc(keyboardfunc);
	glutMotionFunc(motionfunc);
	glutMainLoop();
}
