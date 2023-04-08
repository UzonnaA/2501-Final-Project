#include <stdexcept>
#include <string>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp> 
#include <SOIL/SOIL.h>
#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <chrono>
#include <thread>
#include <string>
#include <random>
#include <path_config.h>


#include "sprite.h"
#include "particles.h"
#include "shader.h"
#include "player_game_object.h"
#include "particle_system.h"
#include "game.h"

namespace game {

// Some configuration constants
// They are written here as global variables, but ideally they should be loaded from a configuration file

// Globals that define the OpenGL window and viewport
const char *window_title_g = "Game Demo";
const unsigned int window_width_g = 800;
const unsigned int window_height_g = 600;
const glm::vec3 viewport_background_color_g(0.0, 0.0, 1.0);
bool UI_on = false;
static std::chrono::time_point<std::chrono::system_clock> game_start_time = std::chrono::system_clock::now();


// Directory with game resources such as textures
const std::string resources_directory_g = RESOURCES_DIRECTORY;


Game::Game(void)
{
    // Don't do work in the constructor, leave it for the Init() function
}


void Game::Init(void)
{

    // Initialize the window management library (GLFW)
    if (!glfwInit()) {
        throw(std::runtime_error(std::string("Could not initialize the GLFW library")));
    }

    // Set window to not resizable
    // Required or else the calculation to get cursor pos to screenspace will be incorrect
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); 

    // Create a window and its OpenGL context
    window_ = glfwCreateWindow(window_width_g, window_height_g, window_title_g, NULL, NULL);
    if (!window_) {
        glfwTerminate();
        throw(std::runtime_error(std::string("Could not create window")));
    }

    // Make the window's OpenGL context the current one
    glfwMakeContextCurrent(window_);

    // Initialize the GLEW library to access OpenGL extensions
    // Need to do it after initializing an OpenGL context
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        throw(std::runtime_error(std::string("Could not initialize the GLEW library: ") + std::string((const char *)glewGetErrorString(err))));
    }

    // Set event callbacks
    glfwSetFramebufferSizeCallback(window_, ResizeCallback);

    // Initialize sprite geometry
    sprite_ = new Sprite();
    sprite_->CreateGeometry();

    // Initialize particle geometry
    particles_ = new Particles();
    particles_->CreateGeometry();

    particles2_ = new Particles();
    particles2_->SetRange(0.02f);
    particles2_->CreateGeometry();
    


    // Initialize particle shader
    particle_shader_.Init((resources_directory_g+std::string("/particle_vertex_shader.glsl")).c_str(), (resources_directory_g+std::string("/particle_fragment_shader.glsl")).c_str());
    particle_shader2_.Init((resources_directory_g + std::string("/particle_vertex_2.glsl")).c_str(), (resources_directory_g + std::string("/particle_fragment_2.glsl")).c_str());


    // Initialize sprite shader
    sprite_shader_.Init((resources_directory_g+std::string("/sprite_vertex_shader.glsl")).c_str(), (resources_directory_g+std::string("/sprite_fragment_shader.glsl")).c_str());

    // Initialize time
    current_time_ = 0.0;


    //ImGui initialization code
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init();





}


Game::~Game()
{
    // Free memory for all objects
    // Only need to delete objects that are not automatically freed
    delete sprite_;
    delete particles_;
    delete particles2_;
    for (int i = 0; i < game_objects_.size(); i++){
        delete game_objects_[i];
    }

    // Close window
    glfwDestroyWindow(window_);
    glfwTerminate();
}


void Game::Setup(void)
{

    // Setup the game world

    // Load textures
    SetAllTextures();


    // Setup the player object (position, texture, vertex count)
    // Note that, in this specific implementation, the player object should always be the first object in the game object vector 
    //The x or y value of an object can be between (-3.5 to 3.5)

    //This code allows for a random value to be generated
    std::random_device rand_device;
    std::mt19937 spawn(rand_device());
    std::uniform_real_distribution<> dis(-3.5, 3.5);


    // Setup the player object (position, texture, vertex count)
    // Note that, in this specific implementation, the player object should always be the first object in the game object vector 
    PlayerGameObject* player1 = new PlayerGameObject(glm::vec3(0.0f, 0.0f, 0.0f), sprite_, &sprite_shader_, tex_[0]);
    game_objects_.push_back(player1);

    // Setup other objects

    for (int i = 0; i < 5; i++) {
        GameObject* enemy1 = new GameObject(glm::vec3(dis(spawn), dis(spawn), 0.0f), sprite_, &sprite_shader_, tex_[1]);
        enemy1->SetType("enemy");
        game_objects_.push_back(enemy1);
    }

    /*GameObject *enemy1 = new GameObject(glm::vec3(dis(spawn), dis(spawn), 0.0f), sprite_, &sprite_shader_, tex_[1]);
    enemy1->SetType("enemy");
    game_objects_.push_back(enemy1);

    GameObject *enemy2 = new GameObject(glm::vec3(1.0f, -0.5f, 0.0f), sprite_, &sprite_shader_, tex_[2]);
    enemy2->SetType("enemy");
    game_objects_.push_back(enemy2);*/

    //Rotating Blade
    GameObject *blade = new GameObject(glm::vec3(0.0f, 0.0f, 0.0f), sprite_, &sprite_shader_, tex_[6]);
    blade->SetParent(player1);
    blade->SetType("blade");
    game_objects_.push_back(blade);


    // Setup background
    // In this specific implementation, the background is always the
    // last object
    GameObject *background = new GameObject(glm::vec3(0.0f, 0.0f, 0.0f), sprite_, &sprite_shader_, tex_[3]);
    background->SetScale(10.0);
    background->SetIsBg(true);
    game_objects_.push_back(background);

    // Setup particle system
    GameObject *particles = new ParticleSystem(glm::vec3(0.0f, -0.5f, 0.0f), particles_, &particle_shader_, tex_[4], game_objects_[0]);
    particles->SetScale(0.2);
    game_objects_.push_back(particles);
}


void Game::ResizeCallback(GLFWwindow* window, int width, int height)
{

    // Set OpenGL viewport based on framebuffer width and height
    glViewport(0, 0, width, height);
}


void Game::SetTexture(GLuint w, const char *fname)
{
    // Bind texture buffer
    glBindTexture(GL_TEXTURE_2D, w);

    // Load texture from a file to the buffer
    int width, height;
    unsigned char* image = SOIL_load_image(fname, &width, &height, 0, SOIL_LOAD_RGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);

    // Texture Wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Texture Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


void Game::SetAllTextures(void)
{
    // Load all textures that we will need
    glGenTextures(NUM_TEXTURES, tex_);
    SetTexture(tex_[0], (resources_directory_g+std::string("/textures/destroyer_red.png")).c_str());
    SetTexture(tex_[1], (resources_directory_g+std::string("/textures/destroyer_green.png")).c_str());
    SetTexture(tex_[2], (resources_directory_g+std::string("/textures/destroyer_blue.png")).c_str());
    SetTexture(tex_[3], (resources_directory_g+std::string("/textures/stars.png")).c_str());
    SetTexture(tex_[4], (resources_directory_g+std::string("/textures/orb.png")).c_str());
    SetTexture(tex_[5], (resources_directory_g + std::string("/textures/bullet.png")).c_str());
    SetTexture(tex_[6], (resources_directory_g + std::string("/textures/blade.png")).c_str());
    glBindTexture(GL_TEXTURE_2D, tex_[0]);
}


void Game::MainLoop(void)
{
    // Loop while the user did not close the window
    double last_time = glfwGetTime();
    while (!glfwWindowShouldClose(window_)){


        



        // Clear background
        glClearColor(viewport_background_color_g.r,
                     viewport_background_color_g.g,
                     viewport_background_color_g.b, 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set view to zoom out, centered by default at 0,0
        float camera_zoom = 0.25f;
        glm::mat4 view_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(camera_zoom, camera_zoom, camera_zoom));

        // Calculate delta time
        double current_time = glfwGetTime();
        double delta_time = current_time - last_time;
        last_time = current_time;
        // Update other events like input handling
        glfwPollEvents();

        // Update the game
        Update(view_matrix, delta_time);

        if (UI_on) {
            //Start UI
            //Start a new ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            //Computing the time
            //Get elapsed time in seconds
            std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
            int elapsed_time = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(now - game_start_time).count());

            //Calculate minutes and seconds
            int minutes = elapsed_time / 60;
            int seconds = elapsed_time % 60;

            //Format time string
            std::string time_str = "Time: " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s";

            

            

            //Menu Text
            std::string KillText = "Kill Count: " + std::to_string(game_objects_[0]->GetKillCount());
            std::string HealthText = "Current Health: " + std::to_string(game_objects_[0]->GetHealth());
            ImGui::Text(time_str.c_str());
            ImGui::Text(HealthText.c_str());
            ImGui::Text(KillText.c_str());



            

            //You can just call Text again to add more text to the GUI
            //ImGui::Text(text.c_str());

            
            //Render the ImGui frame
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window_, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            //End UI
        }


       

        // Push buffer drawn in the background onto the display
        glfwSwapBuffers(window_);

    }
}

bool Game::RayCollision(glm::vec3 start, glm::vec3 direction, glm::vec3 center, float radius) {
    //Quick explanation
    //First I need the direction from the ray start to the center of the circle
    //Then I compute the projection using the dot product
    //Then I find the dist between the ray and the circle center
    //Use the formula from class to know if there should be a collision
    //I'll call this with the needed params later for easier access

    glm::vec3 rayCircleDirection = center - start;
    float projection = glm::dot(rayCircleDirection, direction);

    if (projection < 0.0f) {
        return false;
    }

    glm::vec3 nearestRayPoint = start + projection * direction;
    float dist = glm::distance(nearestRayPoint, center);
    float squaredDist = dist * dist;

    // If the distance is greater than the radius squared, there is no collision
    if (squaredDist > radius * radius) {
        return false;
    }
    else {
        return true;
    }
}


void Game::Update(glm::mat4 view_matrix, double delta_time)
{

    // Update time
    current_time_ += delta_time;

    // Handle user input
    Controls(delta_time);

    //View matrix is updated to follow the player
    glm::vec3 playerPos = dynamic_cast<PlayerGameObject*>(game_objects_[0])->GetPosition();
    view_matrix = glm::translate(view_matrix, -playerPos);

    // Update and render all game objects
    for (int i = 0; i < game_objects_.size(); i++) {

        //These bools will ensure we only deal with bullets and enemies
        bool isBullet = false;
        bool isEnemy = false;


        // Get the current game object
        GameObject* current_game_object = game_objects_[i];

        if (current_game_object->GetType() == "enemy") {
            current_game_object->SetPlayer(playerPos);
        }

        // Update the current game object
        current_game_object->Update(delta_time);
        
        if (current_game_object->CheckIfChild()) {
            current_game_object->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        }

        if (current_game_object->CheckDead()) {
            game_objects_.erase(game_objects_.begin() + i);

            continue;
        }
         

        //First we need the velocity of the bullet
        glm::vec3 direction = current_game_object->GetVelocity();
        direction = glm::normalize(direction);

        if (current_game_object->GetType() == "bullet") {
            isBullet = true;
            //std::cout << current_game_object->GetType() << std::endl;
        }

        // Check for collision with other game objects
        // Note the loop bounds: we avoid testing the last object since
        // it's the background covering the whole game world
        for (int j = i + 1; j < (game_objects_.size()-1); j++) {
            GameObject* other_game_object = game_objects_[j];
            isEnemy = false;

            // Compute distance between object i and object j
            float distance = glm::length(current_game_object->GetPosition() - other_game_object->GetPosition());
            
           

            /*
            The following code is for when the player makes contact with any enemy type objects.

            Current Issues:
            - Weird camera issue when player object is erased?

            */
            if (current_game_object == game_objects_[0] && (other_game_object->GetType() == "enemy")) {
                float distance = glm::length(current_game_object->GetPosition() - other_game_object->GetPosition());
                if (distance < 1.0f) {
                    
                    current_game_object->TakeDamage(1);
                    other_game_object->Kill();
                    current_game_object->IncrementKillCount();
                    if (current_game_object->GetHealth() <= 0) {
                        current_game_object->Kill();
                        glfwSetWindowShouldClose(window_, true);

                    }
                }
            }

            //Above is for normal collision, below will be RayCollision
            
            glm::vec3 center = other_game_object->GetPosition();
            //Just guessing the radius for now, not sure what is actually is yet
            float radius = other_game_object->GetRadius();

            if (other_game_object->GetType() == "enemy") {
                isEnemy = true;
            }

            if (isEnemy && isBullet) {
                if (RayCollision(current_game_object->GetPosition(), direction, center, radius)) {
                    //If I get this far, that means a bullet will eventually hit an enemy
                    //Now we check if the bullet is CURRENTLY HITTING an enemy


                    glm::vec3 bullet_position = current_game_object->GetPosition();
                    glm::vec3 bullet_direction = glm::normalize(current_game_object->GetVelocity());
                    float bullet_speed = glm::length(current_game_object->GetVelocity());
                    glm::vec3 enemy_position = other_game_object->GetPosition();
                    //Just guessing the radius right now
                    float enemy_radius = 0.5f;

                    //The furthest a bullet can travel
                    float bullet_distance = bullet_speed * delta_time;

                    //Find the direction and distance between bullet/enemy
                    glm::vec3 bullet_to_enemy = enemy_position - bullet_position;
                    float bullet_to_enemy_distance = glm::length(bullet_to_enemy);
                    glm::vec3 bullet_to_enemy_direction = glm::normalize(bullet_to_enemy);

                    // Check if the bullet can hit the enemy in the current time frame
                    if (glm::dot(bullet_to_enemy_direction, bullet_direction) >= 0 && bullet_to_enemy_distance <= bullet_distance + enemy_radius) {
                        //We landed a hit in the current time frame
                        current_game_object->Kill();

                        //Other game object is the enemy
                        //I plan on creating an explosion particle effect and stuff

                        // Setup particle system
                        particles3_ = new Particles();
                        particles3_->SetExplode(true);
                        particles3_->CreateGeometry();
                        GameObject* particles = new ParticleSystem(glm::vec3(0.0f, 0.0f, 0.0f), particles3_, &particle_shader_, tex_[4], other_game_object);
                        particles->SetScale(0.2);
                        game_objects_.push_back(particles);
                        

                        //This causes a bug where you can still hit a dead enemy since they don't actually despawn for 2s
                        //I could set a flag that prevents this, but I'll do it later
                        other_game_object->SetMustDie(true,2);
                        
                        game_objects_[0]->IncrementKillCount();

                        std::cout << "Enemy ship shot down!" << std::endl;
                        std::cout << "Player Kills: " + std::to_string(game_objects_[0]->GetKillCount()) << std::endl;
                    }




                }
                
            }
            

            
            

        }

        // Render game object
        current_game_object->Render(view_matrix, current_time_);
    }


    

}


void Game::Controls(double delta_time)
{
    // Get player game object
    GameObject *player = game_objects_[0];
    // Get current position and angle
    glm::vec3 curpos = player->GetPosition();
    float angle = player->GetAngle();
    // Compute current bearing direction
    glm::vec3 dir = player->GetBearing();
    // Adjust motion increment and angle increment 
    // if translation or rotation is too slow
    float speed = delta_time*500.0;
    float motion_increment = 0.001*speed;
    float angle_increment = (glm::pi<float>() / 1800.0f)*speed;

    static std::chrono::time_point<std::chrono::system_clock> current_time, last_bullet_time, last_tab_time;
    static bool first_bullet = true;
    static bool first_tab = true;

    // Check for player input and make changes accordingly
    if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
        player->SetPosition(curpos + motion_increment*dir);
    }
    if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
        player->SetPosition(curpos - motion_increment*dir);
    }
    if (glfwGetKey(window_, GLFW_KEY_TAB) == GLFW_PRESS) {
        //Make it so you can only tab once every 1s
        current_time = std::chrono::system_clock::now();

        if (first_tab || current_time > last_tab_time + std::chrono::milliseconds(300)) {
            UI_on = !UI_on;

            last_tab_time = std::chrono::system_clock::now();
            first_tab = false;
        }

    }
    if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {
        player->SetAngle(angle - angle_increment);
    }
    if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) {
        player->SetAngle(angle + angle_increment);
    }
    if (glfwGetKey(window_, GLFW_KEY_Z) == GLFW_PRESS) {
        player->SetPosition(curpos - motion_increment*player->GetRight());
    }
    if (glfwGetKey(window_, GLFW_KEY_C) == GLFW_PRESS) {
        player->SetPosition(curpos + motion_increment*player->GetRight());
    }
    if (glfwGetKey(window_, GLFW_KEY_Q) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window_, true);
    }
    if (glfwGetKey(window_, GLFW_KEY_F) == GLFW_PRESS) {
        //Create a bullet
        //Edit all of its properties so it fires correctly
        //Push bullet
        //Call update
        current_time = std::chrono::system_clock::now();

        //if(player->GetWeapon()=="bullet") {
        if (first_bullet || current_time > last_bullet_time + std::chrono::milliseconds(500)) {
            GameObject* bullet = new GameObject(player->GetPosition(), sprite_, &sprite_shader_, tex_[5]);
            

            bullet->SetScale(0.5);
            bullet->SetMustDie(true, 15);
            bullet->SetAngle(player->GetAngle());
            bullet->SetVelocity(5.0f * player->GetBearing());
            bullet->SetType("bullet");


            
            game_objects_.insert(game_objects_.begin() + 1, bullet);


            // Setup particle system
            GameObject* particles = new ParticleSystem(glm::vec3(0.0f, -0.3f, 0.0f), particles2_, &particle_shader2_, tex_[4], game_objects_[1]);
            particles->SetScale(0.2);
            game_objects_.push_back(particles);

            //std::cout << "BULLET FIRED" << std::endl;

            bullet->Update(delta_time);

            last_bullet_time = std::chrono::system_clock::now();
            first_bullet = false;


        }
        //}

        //if(player->GetWeapon()=="aoe") {
        
        //}

        //if(player->GetWeapon()=="undecided") {
        
        //}
        

        
    }
}
       
} // namespace game
