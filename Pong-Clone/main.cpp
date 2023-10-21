/**
* Author: Jaden Thakur
* Assignment: Pong Clone
* Date due: 2023-10-21, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


// #defines
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define LOG(argument) std::cout << argument << std::endl;
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1

// includes
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include <vector>


// useful gloabal variables
    const float MILLISECONDS_IN_SECOND = 1000.0f;
    float g_previous_ticks = 0.0f;


// necessary gloabal variables
    bool g_game_is_running = true;
    ShaderProgram g_program;


// display window variables
    SDL_Window* displayWindow;
    const int WINDOW_HEIGHT = 600,
              WINDOW_WIDTH  = 1000;

// Background Color (purple)
    const float BG_RED      = 0.192f,
                BG_GREEN    = 0.039f,
                BG_BLUE     = 0.192f,
                BG_ALPHA    = 1.0f;

// Text and Mid Line Color (green)
    const float TXT_RED     = 0.639f,
                TXT_GREEN   = 0.969f,
                TXT_BLUE    = 0.710f,
                TXT_ALPHA   = 1.0f;


// Viewport
    const int VIEWPORT_X      = 0,
              VIEWPORT_Y      = 0,
              VIEWPORT_WIDTH  = WINDOW_WIDTH,
              VIEWPORT_HEIGHT = WINDOW_HEIGHT;

    glm::mat4 g_view_matrix, // cam orientation
              g_projection_matrix; //cam characteristics
              



// Textures
    const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl";
    const char F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

    const char ball_texture[] = "assets/pumpkin.png",
               paddle1_texture[] = "assets/skeleton-LEFT.png",
               paddle2_texture[] = "assets/skeleton-RIGHT.png";

    GLuint ball_texture_id,
           paddle1_texture_id,
           paddle2_texture_id;


    const GLint NUMBER_OF_TEXTURES = 1,
                LEVEL_OF_DETAIL = 0,
                TEXTURE_BORDER  = 0;


// Paddle and Ball Matrices
    glm::mat4 g_paddle1_matrix,
              g_paddle2_matrix,
              g_ball_matrix;
// Paddle and Ball Vectors
    glm::vec3 g_player_position     = glm::vec3(-4.0f, 0.0f, 0.0f),
              g_computer_position   = glm::vec3(4.0f, 0.0f, 0.0f),
              g_ball_position       = glm::vec3(0.0f, 0.0f, 0.0f),
              g_player_movement     = glm::vec3(0.0f, 0.0f, 0.0f),
              g_computer_movement   = glm::vec3(0.0f, 0.0f, 0.0f),
              g_ball_movement       = glm::vec3(0.0f, 0.0f, 0.0f),
              g_player_scale        = glm::vec3(1.0f, 2.0f, 1.0f),
              g_computer_scale      = glm::vec3(1.0f, 2.0f, 1.0f),
              g_ball_scale          = glm::vec3(0.4f, 0.4f, 1.0f);
              


// Gamer State Variables
    bool g_won;
    bool g_game_over = false;

// Player Variables
    int g_player_score = 0;
    float g_player_speed = 100.0f,
          g_player_height = g_player_scale.y,
          g_player_width = g_player_scale.x,
          g_player_max_y,
          g_player_max_x;

// Ball Variables
    float g_ball_speed = 1.0f,
          g_ball_height = g_ball_scale.y,
          g_ball_width = g_ball_scale.x,
          g_ball_max_y,
          g_ball_max_x;

// Computer Variables
    int g_computer_score = 0;
    float g_computer_speed = 1.0f,
          g_computer_height = g_computer_scale.y,
          g_computer_width = g_computer_scale.x,
          g_computer_max_y,
          g_computer_max_x; 

              

GLuint load_texture(const char* filepath) {

    // load image
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL) {
        LOG("Unable to load image");
        assert(false);
    }


    // Bind texture
    GLuint texture_ID;
    glGenTextures(NUMBER_OF_TEXTURES, &texture_ID);
    glBindTexture(GL_TEXTURE_2D, texture_ID);
    glTexImage2D(
        GL_TEXTURE_2D, 
        LEVEL_OF_DETAIL, 
        GL_RGBA,
        width, height,
        TEXTURE_BORDER, 
        GL_RGBA, 
        GL_UNSIGNED_BYTE,
        image
    );


    //sets texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // release image from memory
    stbi_image_free(image);
    return texture_ID;
}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id) {
    g_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}


void initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Pong Clone", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // initialize viewport
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    // load texture shaders
    g_program.load(V_SHADER_PATH, F_SHADER_PATH);

    float rel_width = 5.0f,
          rel_height  = 3.75f;

    // initialize values of view matrix and projection matrix as well as model matrix
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-rel_width, rel_width, -rel_height, rel_height, -1.0f, 1.0f);
    
    // set view and projection matrices
    g_program.set_view_matrix(g_view_matrix);
    g_program.set_projection_matrix(g_projection_matrix);
    

    // initialize object matrix values
    g_paddle1_matrix = glm::mat4(1.0f);
    g_paddle2_matrix = glm::mat4(1.0f);
    g_ball_matrix = glm::mat4(1.0f);

    // load all textures into texture IDs
    paddle1_texture_id = load_texture(paddle1_texture);
    paddle2_texture_id = load_texture(paddle2_texture);
    ball_texture_id = load_texture(ball_texture);


    // i forgot what these do, ask professor
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    // set up iniitial positons
    g_paddle1_matrix = glm::translate(g_paddle1_matrix, g_player_position);
    g_paddle2_matrix = glm::translate(g_paddle2_matrix, g_computer_position);
    g_ball_matrix = glm::translate(g_ball_matrix, g_ball_position);


    // sets ball size
    g_ball_matrix = glm::scale(g_ball_matrix, g_ball_scale);
    g_paddle1_matrix = glm::scale(g_paddle1_matrix, g_player_scale);
    g_paddle2_matrix = glm::scale(g_paddle2_matrix, g_computer_scale);

    g_ball_max_y = rel_height - (g_ball_height / 2);
    g_ball_max_x = rel_width - (g_ball_width / 2);
    g_player_max_y = rel_height - (g_player_height / 2);
    g_player_max_x = rel_width - (g_player_width / 2);

    // sets ball initial direction
    g_ball_movement.x = -1.0f;
    g_ball_movement.y = 1.0f;
        
    // set the shader program
    glUseProgram(g_program.get_program_id()); 
    // set background
    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_ALPHA);
    

}


void process_input() {

    // stop player movement with no input
    g_player_movement = glm::vec3(0.0f);

    // create click event
    SDL_Event event;
    
    // create loop to detect events
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            // quit
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_w:
                g_player_movement.y = 1.0f;
                break;

            case SDLK_s:
                g_player_movement.y = -1.0f;
                break;

            case SDLK_ESCAPE:
                g_game_is_running = false;
                break;

            default:
                break;
            }
        default:
            break;
        }

        const Uint8* key_state = SDL_GetKeyboardState(NULL);
        if (key_state[SDL_SCANCODE_W])
        {
            g_player_movement.y = 1.0f;
        }
        else if (key_state[SDL_SCANCODE_S])
        {
            g_player_movement.y = -1.0f;
        }

    }


    

    // stops player from cheating movement
    if (glm::length(g_player_movement) > 1.0f)
    {
        g_player_movement = glm::normalize(g_player_movement);
    }
}


const int FONTBANK_SIZE = 16;

GLuint player_score_id; 
GLuint computer_score_id;
float screen_size = WINDOW_HEIGHT;
float spacing = 2;

void DrawText(ShaderProgram* program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for each character
    // Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their position
        //    relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (screen_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    g_program.set_model_matrix(model_matrix);
    glUseProgram(program->get_program_id());

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}
 
void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    
    // Vertices (these always stay the same unless u want to enlarge the rectanlge but the coors should not move from the origin)
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };


    glVertexAttribPointer(g_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_program.get_position_attribute());

    glVertexAttribPointer(g_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_program.get_tex_coordinate_attribute());

    // Bind textures
    g_program.set_model_matrix(g_paddle1_matrix);
    glBindTexture(GL_TEXTURE_2D, paddle1_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); 
    g_program.set_model_matrix(g_paddle2_matrix);
    glBindTexture(GL_TEXTURE_2D, paddle2_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    g_program.set_model_matrix(g_ball_matrix);
    glBindTexture(GL_TEXTURE_2D, ball_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_program.get_position_attribute());
    glDisableVertexAttribArray(g_program.get_tex_coordinate_attribute());


    SDL_GL_SwapWindow(displayWindow);
}


void update() {

    glClear(GL_COLOR_BUFFER_BIT);
    

    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    // player movement logic
    if (g_player_position.y >= -g_player_max_y && g_player_position.y <= g_player_max_y) {
        g_player_position += g_player_movement * g_player_speed * (delta_time);
    }
    else if (g_player_position.y <= -g_player_max_y) {
        g_player_position.y = -g_player_max_y;
    }
    else if (g_player_position.y >= g_player_max_y) {
        g_player_position.y = g_player_max_y;
    }

    
    // computer movement logic
    if ((g_ball_position.y > g_computer_position.y) && g_computer_position.y <= g_computer_max_y) {
        g_computer_movement.y = g_ball_movement.y;
        g_computer_position.y += g_computer_movement.y * g_computer_speed * delta_time;
    }
    if ((g_ball_position.y < g_computer_position.y) && g_computer_position.y >= -g_computer_max_y) {
        g_computer_movement.y = g_ball_movement.y;
        g_computer_position.y += g_computer_movement.y * g_computer_speed * delta_time;
    }
    if (g_computer_position.y < -g_computer_max_y){
        g_computer_position.y = -g_computer_max_y;
        g_computer_movement.y = 0.0f;
    }
    if (g_computer_position.y > g_computer_max_y) {
        g_computer_position.y = g_computer_max_y;
        g_computer_movement.y = 0.0f;
    }
    

    // ball logic
    g_ball_position += g_ball_movement * g_ball_speed * (delta_time);

    if (g_ball_position.y < -g_ball_max_y) {
        g_ball_movement.y = -g_ball_movement.y; 
    }
    if (g_ball_position.y > g_ball_max_y) {
        g_ball_movement.y = -g_ball_movement.y;   
    }
    if (g_ball_position.x < -g_ball_max_x) {
        g_computer_score++;
        LOG("Player: " << g_player_score << " Comp: " << g_computer_score);
        g_ball_movement.x = -g_ball_movement.x;
        g_ball_position.x = 0;
    }
    if (g_ball_position.x > g_ball_max_x) {
        g_player_score++;
        LOG("Player: " << g_player_score << " Comp: " << g_computer_score);
        g_ball_movement.x = -g_ball_movement.x;
        g_ball_position.x = 0;
    }
   
    g_paddle1_matrix = glm::mat4(1.0f);
    g_paddle2_matrix = glm::mat4(1.0f);
    g_ball_matrix = glm::mat4(1.0f);

    g_paddle1_matrix = glm::translate(g_paddle1_matrix, g_player_position);
    g_paddle2_matrix = glm::translate(g_paddle2_matrix, g_computer_position);
    g_ball_matrix = glm::translate(g_ball_matrix, g_ball_position);

    g_ball_matrix = glm::scale(g_ball_matrix, g_ball_scale);
    g_paddle1_matrix = glm::scale(g_paddle1_matrix, g_player_scale);
    g_paddle2_matrix = glm::scale(g_paddle2_matrix, g_computer_scale);

    if (g_game_over) {

    }
}

void shutdown() {
    SDL_Quit();
}


int main(int argc, char* argv[]) {
    initialize();
    
    while (g_game_is_running) {
        process_input();
        update();
        render();
    }

    shutdown();
    
    return 0;
}