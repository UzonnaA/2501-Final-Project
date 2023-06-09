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
    const char* window_title_g = "Game Demo";
    const unsigned int window_width_g = 1100;
    const unsigned int window_height_g = 800;
    const glm::vec3 viewport_background_color_g(0.0, 0.0, 1.0);
    bool UI_on = false;
    int minigunAmmoCount = 50;   // global variable denoting the amount of ammo you minigun is currently holding
    bool game_is_over = false;
    bool last_frame = false;
    int game_speed = 1;
    std::string survival_time = "N/A";
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
            throw(std::runtime_error(std::string("Could not initialize the GLEW library: ") + std::string((const char*)glewGetErrorString(err))));
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
        particle_shader_.Init((resources_directory_g + std::string("/particle_vertex_shader.glsl")).c_str(), (resources_directory_g + std::string("/particle_fragment_shader.glsl")).c_str());
        particle_shader2_.Init((resources_directory_g + std::string("/particle_vertex_2.glsl")).c_str(), (resources_directory_g + std::string("/particle_fragment_2.glsl")).c_str());


        // Initialize sprite shader
        sprite_shader_.Init((resources_directory_g + std::string("/sprite_vertex_shader.glsl")).c_str(), (resources_directory_g + std::string("/sprite_fragment_shader.glsl")).c_str());

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
        for (int i = 0; i < game_objects_.size(); i++) {
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
        /*std::random_device rand_device;
        std::mt19937 spawn(rand_device());
        std::uniform_real_distribution<> dis(-3.5, 3.5);*/


        // Setup the player object (position, texture, vertex count)
        // Note that, in this specific implementation, the player object should always be the first object in the game object vector 
        PlayerGameObject* player1 = new PlayerGameObject(glm::vec3(0.0f, -2.0f, 0.0f), sprite_, &sprite_shader_, tex_[0]);
        player1->SetType("player");
        player1->SetGoldShip(tex_[12]);
        game_objects_.push_back(player1);

        // Setup other objects

        /*for (int i = 0; i < 5; i++) {a
            GameObject* enemy1 = new GameObject(glm::vec3(dis(spawn), dis(spawn), 0.0f), sprite_, &sprite_shader_, tex_[1]);
            enemy1->SetType("enemy");
            game_objects_.push_back(enemy1);

        }*/

        /*GameObject *enemy1 = new GameObject(glm::vec3(dis(spawn), dis(spawn), 0.0f), sprite_, &sprite_shader_, tex_[1]);
        enemy1->SetType("enemy");
        game_objects_.push_back(enemy1);

        GameObject *enemy2 = new GameObject(glm::vec3(1.0f, -0.5f, 0.0f), sprite_, &sprite_shader_, tex_[2]);
        enemy2->SetType("enemy");
        game_objects_.push_back(enemy2);*/

        //Rotating Blade
        GameObject* blade = new GameObject(glm::vec3(0.0f, 0.0f, 0.0f), sprite_, &sprite_shader_, tex_[6]);
        blade->SetParent(player1);
        blade->SetType("blade");
        game_objects_.push_back(blade);


        // Setup background
        // In this specific implementation, the background is always the
        // last object
        GameObject* background = new GameObject(glm::vec3(0.0f, 0.0f, 0.0f), sprite_, &sprite_shader_, tex_[3]);
        background->SetScale(1000.0);
        background->SetIsBg(true);
        game_objects_.push_back(background);

        // Setup particle system
        GameObject* particles = new ParticleSystem(glm::vec3(0.0f, -0.5f, 0.0f), particles_, &particle_shader_, tex_[4], game_objects_[0]);
        particles->SetScale(0.2);
        game_objects_.push_back(particles);
    }


    void Game::ResizeCallback(GLFWwindow* window, int width, int height)
    {

        // Set OpenGL viewport based on framebuffer width and height
        glViewport(0, 0, width, height);
    }


    void Game::SetTexture(GLuint w, const char* fname)
    {
        // Bind texture buffer
        glBindTexture(GL_TEXTURE_2D, w);

        // Load texture from a file to the buffer
        int width, height;
        unsigned char* image = SOIL_load_image(fname, &width, &height, 0, SOIL_LOAD_RGBA);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        SOIL_free_image_data(image);

        // Texture Wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // Texture Filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }


    void Game::SetAllTextures(void)
    {
        // Load all textures that we will need
        glGenTextures(NUM_TEXTURES, tex_);
        SetTexture(tex_[0], (resources_directory_g + std::string("/textures/destroyer_red.png")).c_str());
        SetTexture(tex_[1], (resources_directory_g + std::string("/textures/destroyer_green.png")).c_str());
        SetTexture(tex_[2], (resources_directory_g + std::string("/textures/destroyer_blue.png")).c_str());
        SetTexture(tex_[3], (resources_directory_g + std::string("/textures/stars.png")).c_str());
        SetTexture(tex_[4], (resources_directory_g + std::string("/textures/orb.png")).c_str());
        SetTexture(tex_[5], (resources_directory_g + std::string("/textures/bullet.png")).c_str());
        SetTexture(tex_[6], (resources_directory_g + std::string("/textures/blade.png")).c_str());
        SetTexture(tex_[7], (resources_directory_g + std::string("/textures/aoeSprite.png")).c_str());  //need to change texture
        SetTexture(tex_[8], (resources_directory_g + std::string("/textures/minigun.png")).c_str());  
        
        //Textures for collectibles
        SetTexture(tex_[9], (resources_directory_g + std::string("/textures/Star.png")).c_str()); //Star
        SetTexture(tex_[10], (resources_directory_g + std::string("/textures/ammo.png")).c_str()); //Ammo 
        SetTexture(tex_[11], (resources_directory_g + std::string("/textures/heart.png")).c_str()); //Heart
        SetTexture(tex_[12], (resources_directory_g + std::string("/textures/golden_ship.png")).c_str()); //We will switch to this spaceship when we gold invincible

        //Textures for new enemies
        SetTexture(tex_[13], (resources_directory_g + std::string("/textures/red_enemy.png")).c_str()); 
        SetTexture(tex_[14], (resources_directory_g + std::string("/textures/white_enemy.png")).c_str()); 
        SetTexture(tex_[15], (resources_directory_g + std::string("/textures/blue_enemy.png")).c_str());

        glBindTexture(GL_TEXTURE_2D, tex_[0]);
    }

    
    
    void Game::MainLoop(void)
    {
        // Loop while the user did not close the window
        double last_time = glfwGetTime();
        while (!glfwWindowShouldClose(window_)) {






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


            //Spawn the enemies
            static std::chrono::time_point<std::chrono::system_clock> last_spawn_time, current_time2;
            static bool first_wave = true;
            current_time2 = std::chrono::system_clock::now();
            static int tick = 0;

            if (first_wave || current_time2 > last_spawn_time + std::chrono::milliseconds(7000 - game_speed * 400)) {
                //std::cout << "ENEMIES SPAWNED" << std::endl;
                SpawnEnemies(game_objects_[0]->GetPosition());
                first_wave = false;
                last_spawn_time = std::chrono::system_clock::now();
                tick += 1;
            }

            if (tick == 4) {
                game_speed += 1;
                tick = 0;
            }

            //Spawn Collectibles
            static std::chrono::time_point<std::chrono::system_clock> last_collectible_time;
            static bool first_collectible = true;
            
            current_time2 = std::chrono::system_clock::now();
            

            if (first_collectible || current_time2 > last_collectible_time + std::chrono::milliseconds(4000)) {
                //std::cout << "ENEMIES SPAWNED" << std::endl;
                SpawnCollectibles(game_objects_[0]->GetPosition());
                first_collectible = false;
                last_collectible_time = std::chrono::system_clock::now();
            }

            




            // Update the game
            Update(view_matrix, delta_time);

            //Computing the time
            //Get elapsed time in seconds
            std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
            int elapsed_time = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(now - game_start_time).count());

            //Calculate minutes and seconds
            int minutes = elapsed_time / 60;
            int seconds = elapsed_time % 60;

            //Format time string
            std::string time_str = "Time: " + std::to_string(minutes) + "m " + std::to_string(seconds) + "s";
            survival_time = std::to_string(minutes) + "m " + std::to_string(seconds) + "s";

            if (UI_on) {
                //Start UI
                //Start a new ImGui frame
                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

               





                //Menu Text
                std::string KillText = "Kill Count: " + std::to_string(game_objects_[0]->GetKillCount());
                std::string HealthText = "Current Health: " + std::to_string(game_objects_[0]->GetHealth());
                std::string MinigunAmmoText = "Minigun Ammo: " + std::to_string((minigunAmmoCount));
                ImGui::Text(time_str.c_str());
                ImGui::Text(HealthText.c_str());
                ImGui::Text(KillText.c_str());
                ImGui::Text(MinigunAmmoText.c_str());

                //When the game ends...
                static int finalKills = 0;
                static int finalMinutes = 0;
                static int finalSeconds = 0;
                static int finalScore = 0;

                if (game_is_over && !last_frame) {
                    finalKills = game_objects_[0]->GetKillCount();
                    finalMinutes = minutes;
                    finalSeconds = seconds;
                    finalScore = (finalSeconds * 10) + (finalMinutes * 60) + (game_speed * 100) + (finalKills * 100);
                    last_frame = true;
                }

                if (game_is_over) {
                    ImGui::EndFrame();
                    ImGui::NewFrame();
                    ImGui::Text("Game Over!");
                    std::string KillText = "Total Kills: " + std::to_string(finalKills);
                    std::string TimeText = "Time Survived: " + std::to_string(finalMinutes) + "m " + std::to_string(finalSeconds) + "s";
                    std::string ScoreText = "FINAL SCORE: " + std::to_string(finalScore) + " points";
                    ImGui::Text(KillText.c_str());
                    ImGui::Text(TimeText.c_str());
                    ImGui::Text(ScoreText.c_str());
                    ImGui::Text("Press ESC to close the game.");
                }





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


    void Game::SpawnEnemies(glm::vec3 playerPos) {
        std::random_device rand_device;
        std::mt19937 spawn(rand_device());
        std::uniform_real_distribution<> dis(-3.5, 3.5);

        

        
        for (int i = 0; i < 4 + game_speed; i++) {

            std::uniform_real_distribution<> col(1, 11);
            int type = static_cast<int>(col(spawn));
            
            if (type >= 1 && type < 7) {
                //60% chance to spawn normal bullet enemy
                GameObject* enemy1 = new GameObject(glm::vec3(dis(spawn), playerPos.y + 7.0f, 0.0f), sprite_, &sprite_shader_, tex_[13]);
                enemy1->SetType("enemy");
                enemy1->InitFiring(sprite_, &sprite_shader_, tex_[5], game_objects_, 1);
                game_objects_.insert(game_objects_.begin() + 1, enemy1);
            }

            if (type >= 7 && type < 10) {
                //30% chance to spawn aoe bullet enemy
                GameObject* enemy1 = new GameObject(glm::vec3(dis(spawn), playerPos.y + 7.0f, 0.0f), sprite_, &sprite_shader_, tex_[14]);
                enemy1->SetType("enemy");
                enemy1->InitFiring(sprite_, &sprite_shader_, tex_[7], game_objects_, 2);
                game_objects_.insert(game_objects_.begin() + 1, enemy1);
            }

            if (type >= 10) {
                //10% chance to spawn minigun enemy
                GameObject* enemy1 = new GameObject(glm::vec3(dis(spawn), playerPos.y + 7.0f, 0.0f), sprite_, &sprite_shader_, tex_[15]);
                enemy1->SetType("enemy");
                enemy1->InitFiring(sprite_, &sprite_shader_, tex_[8], game_objects_, 3);
                game_objects_.insert(game_objects_.begin() + 1, enemy1);
            }


        }

       


        

    }

    void Game::SpawnCollectibles(glm::vec3 playerPos) {

        //1 is invincibility (star)
        //2 is more minigun ammo (ammo)
        //3 is +1 health (heart)


        std::random_device rand_device;
        std::mt19937 spawn(rand_device());
        std::uniform_real_distribution<> dis(-3.5, 3.5);
        std::uniform_real_distribution<> col(1, 4);

        int type = static_cast<int>(col(spawn));

        if (type == 1) {
            GameObject* collectible = new GameObject(glm::vec3(dis(spawn), playerPos.y + 7.0f, 0.0f), sprite_, &sprite_shader_, tex_[9]);
            collectible->SetType("star");
            collectible->SetScale(0.5);
            //collectible->InitFiring(sprite_, &sprite_shader_, tex_[5], game_objects_, 1);
            game_objects_.insert(game_objects_.begin() + 1, collectible);
        }

        if (type == 2) {
            GameObject* collectible = new GameObject(glm::vec3(dis(spawn), playerPos.y + 7.0f, 0.0f), sprite_, &sprite_shader_, tex_[10]);
            collectible->SetType("ammo");
            collectible->SetScale(0.5);
            //collectible->InitFiring(sprite_, &sprite_shader_, tex_[5], game_objects_, 1);
            game_objects_.insert(game_objects_.begin() + 1, collectible);
        }

        if (type == 3) {
            GameObject* collectible = new GameObject(glm::vec3(dis(spawn), playerPos.y + 7.0f, 0.0f), sprite_, &sprite_shader_, tex_[11]);
            collectible->SetType("heart");
            collectible->SetScale(0.5);
            //collectible->InitFiring(sprite_, &sprite_shader_, tex_[5], game_objects_, 1);
            game_objects_.insert(game_objects_.begin() + 1, collectible);
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
        glm::vec3 offset = glm::vec3(0.0f, 2.0f, 0.0f);

        //view_matrix = glm::translate(view_matrix, -playerPos - offset);

        glm::vec3 cameraPos = glm::vec3(0.0f, playerPos.y, 0.0f);
        view_matrix = glm::translate(view_matrix, -cameraPos - offset);







        // Update and render all game objects
        for (int i = 0; i < game_objects_.size(); i++) {

            //These bools will ensure we only deal with bullets, aoe and enemies
            bool isBullet = false;
            bool isEnemy = false;
            bool isAoe = false;
            bool isMini = false;

            // Get the current game object
            GameObject* current_game_object = game_objects_[i];

            if (current_game_object->GetType() == "enemy") {    //for enemy states
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

            if (current_game_object->CheckGhost()) {
                //if we're ghosted, stop collision
                current_game_object->Render(view_matrix, current_time_);
                continue;
            }


            //First we need the velocity of the bullet      /other projectile
            glm::vec3 direction = current_game_object->GetVelocity();
            direction = glm::normalize(direction);

            if (current_game_object->GetType() == "bullet") {
                isBullet = true;
                //std::cout << current_game_object->GetType() << std::endl;
            }

            if (current_game_object->GetType() == "aoe") {
                isAoe = true;
            }

            if (current_game_object->GetType() == "minigun") {
                isMini = true;
            }

            // Check for collision with other game objects
            // Note the loop bounds: we avoid testing the last object since
            // it's the background covering the whole game world
            for (int j = i + 1; j < (game_objects_.size() - 1); j++) {
                GameObject* other_game_object = game_objects_[j];
                isEnemy = false;

                if (other_game_object->CheckGhost()) {
                    //if we're ghosted, stop collision
                    continue;
                }

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
                            //current_game_object->Kill();
                            
                            game_is_over = true;
                            UI_on = true;
                            //glfwSetWindowShouldClose(window_, true);
                            

                        }
                    }
                }

                //The following will allow enemy bullets to collide with the player
                if (current_game_object == game_objects_[0] && (other_game_object->GetType() == "enemyBullet")) {
                    float distance = glm::length(current_game_object->GetPosition() - other_game_object->GetPosition());
                    if (distance < 1.0f) {

                        current_game_object->TakeDamage(1);
                        other_game_object->Kill();
                        if (current_game_object->GetHealth() <= 0) {
                            //current_game_object->Kill();
                            game_is_over = true;
                            UI_on = true;
                            //glfwSetWindowShouldClose(window_, true);

                        }
                    }
                }


                //The following code is collectible collision
                if (current_game_object == game_objects_[0] && (other_game_object->GetType() == "star")) {
                    float distance = glm::length(current_game_object->GetPosition() - other_game_object->GetPosition());
                    if (distance < 1.0f) {

                        current_game_object->SetStarCount(current_game_object->GetStarCount() + 1);
                        other_game_object->Kill();
                        
                    }
                }

                if (current_game_object == game_objects_[0] && (other_game_object->GetType() == "ammo")) {
                    float distance = glm::length(current_game_object->GetPosition() - other_game_object->GetPosition());
                    if (distance < 1.0f) {

                        minigunAmmoCount += 10;
                        //if (minigunAmmoCount >= 50) {
                            //minigunAmmoCount = 50;
                        //}
                        other_game_object->Kill();

                    }
                }

                if (current_game_object == game_objects_[0] && (other_game_object->GetType() == "heart")) {
                    float distance = glm::length(current_game_object->GetPosition() - other_game_object->GetPosition());
                    if (distance < 1.0f) {

                        current_game_object->SetHealth(current_game_object->GetHealth()+1);
                        //if (current_game_object->GetHealth() >= 5) {
                            //current_game_object->SetHealth(5);
                        //}
                        other_game_object->Kill();

                    }
                }






                //Above is for normal collision, below will be RayCollision

                glm::vec3 center = other_game_object->GetPosition();
                //Just guessing the radius for now, not sure what is actually is yet
                float radius = other_game_object->GetRadius();

                if (other_game_object->GetType() == "enemy") {
                    isEnemy = true;
                }

                if (isEnemy && (isBullet || isAoe || isMini)) { //updated so collision works with both bullet and aoe atk

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
                            GameObject* particles = new ParticleSystem(glm::vec3(0.0f, 0.0f, 0.0f), particles3_, &particle_shader2_, tex_[4], other_game_object);
                            particles->SetScale(0.2);
                            game_objects_.push_back(particles);


                            //This causes a bug where you can still hit a dead enemy since they don't actually despawn for 2s
                            //I could set a flag that prevents this, but I'll do it later
                            other_game_object->SetGhost(true);
                            other_game_object->SetMustDie(true, 1);

                            game_objects_[0]->IncrementKillCount();

                            //std::cout << "Enemy ship shot down!" << std::endl;
                            //std::cout << "Player Kills: " + std::to_string(game_objects_[0]->GetKillCount()) << std::endl;
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
        GameObject* player = game_objects_[0];
        // Get current position and angle
        glm::vec3 curpos = player->GetPosition();
        float angle = player->GetAngle();
        // Compute current bearing direction
        glm::vec3 dir = player->GetBearing();
        // Adjust motion increment and angle increment 
        // if translation or rotation is too slow
        float speed = delta_time * 500.0;
        float motion_increment = 0.001 * speed;
        float angle_increment = (glm::pi<float>() / 1800.0f) * speed;
        //aaint minigunAmmoCount = 0;
        static std::chrono::time_point<std::chrono::system_clock> current_time, last_bullet_time, last_tab_time, last_aoe_time, last_switch_time, last_minigun_time;    //edited to also have aoe and weapon switch
        static bool first_bullet = true;
        static bool first_aoe = true;
        static bool first_minigun = true;
        static bool first_switch = true;

        static bool first_tab = true;
        player->SetPosition(curpos + motion_increment * game_speed * dir);

        // Check for player input and make changes accordingly
        if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
            //player->SetPosition(curpos + motion_increment*dir);
        }
        if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
            //player->SetPosition(curpos - motion_increment * dir);
        }
        if (glfwGetKey(window_, GLFW_KEY_TAB) == GLFW_PRESS) {
            //Make it so you can only tab once every 1s (ish)
            current_time = std::chrono::system_clock::now();

            if (first_tab || current_time > last_tab_time + std::chrono::milliseconds(300)) {
                UI_on = !UI_on;

                last_tab_time = std::chrono::system_clock::now();
                first_tab = false;
            }

            if (game_is_over) {
                UI_on = true;
            }

        }
        if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {


            //player->SetPosition(curpos + motion_increment * 2 * player->GetRight());
            player->SetVelocity(player->GetVelocity() + motion_increment * 5 * player->GetRight());
            //std::cout << "(" << player->GetVelocity().x  << "," << player->GetVelocity().y << ")" << std::endl;

        }
        if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) {
            // player->SetPosition(curpos - motion_increment * 2 * player->GetRight());
            player->SetVelocity(player->GetVelocity() - motion_increment * 5 * player->GetRight());
        }
        if (glfwGetKey(window_, GLFW_KEY_Z) == GLFW_PRESS) {
            player->SetPosition(curpos - motion_increment * 2 * player->GetRight());
        }
        if (glfwGetKey(window_, GLFW_KEY_C) == GLFW_PRESS) {
            player->SetPosition(curpos + motion_increment * player->GetRight());
        }
        if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window_, true);
        }
        if (glfwGetKey(window_, GLFW_KEY_Q) == GLFW_PRESS) {
            player->SetAngle(angle + angle_increment);

        }
        if (glfwGetKey(window_, GLFW_KEY_E) == GLFW_PRESS) {
            player->SetAngle(angle - angle_increment);
        }

        if (glfwGetKey(window_, GLFW_KEY_R) == GLFW_PRESS) {    //makes it so you can only switch every 600ms

            current_time = std::chrono::system_clock::now();

            if (first_switch || current_time > last_switch_time + std::chrono::milliseconds(600)) {
                player->IncrementWeaponType();

                last_switch_time = std::chrono::system_clock::now();
                first_switch = false;
            }
        }

        if (glfwGetKey(window_, GLFW_KEY_F) == GLFW_PRESS) {
            //Create a bullet
            //Edit all of its properties so it fires correctly
            //Push bullet
            //Call update
            current_time = std::chrono::system_clock::now();

            if (player->GetWeaponType() == 1) {
                if (first_bullet || current_time > last_bullet_time + std::chrono::milliseconds(850)) {
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
            }

            if (player->GetWeaponType() == 2) {         //sometimes edges of aoe sprite do not count as a connection

                if (first_aoe || current_time > last_aoe_time + std::chrono::milliseconds(2000)) {
                    GameObject* aoe = new GameObject(player->GetPosition(), sprite_, &sprite_shader_, tex_[7]); //need to change texture 

                    aoe->SetScale(1.5);
                    aoe->SetMustDie(true, 15);
                    aoe->SetAngle(player->GetAngle());
                    aoe->SetVelocity(5.0f * player->GetBearing());
                    aoe->SetType("aoe");



                    game_objects_.insert(game_objects_.begin() + 1, aoe);

                    aoe->Update(delta_time);

                    last_aoe_time = std::chrono::system_clock::now();
                    first_aoe = false;
                }
            }

            if (player->GetWeaponType() == 3) {
                if (first_minigun || current_time > last_minigun_time + std::chrono::milliseconds(200)) {
                    if (minigunAmmoCount > 0) {
                        GameObject* minigun = new GameObject(player->GetPosition(), sprite_, &sprite_shader_, tex_[8]); //need to change texture 

                        minigun->SetScale(.15);
                        minigun->SetMustDie(true, 15);
                        minigun->SetAngle(player->GetAngle());
                        minigun->SetVelocity(5.0f * player->GetBearing());
                        minigun->SetType("minigun");



                        game_objects_.insert(game_objects_.begin() + 1, minigun);

                        minigun->Update(delta_time);

                        last_minigun_time = std::chrono::system_clock::now();
                        first_minigun = false;
                        minigunAmmoCount--;
                    }
                }
            }



        }
    }

} // namespace game
