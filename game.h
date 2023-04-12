#ifndef GAME_H_
#define GAME_H_

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "shader.h"
#include "game_object.h"

namespace game {

    // A class for holding the main game objects
    class Game {

        public:
            // Constructor and destructor
            Game(void);
            ~Game();

            // Call Init() before calling any other method
            // Initialize graphics libraries and main window
            void Init(void); 

            // Set up the game (scene, game objects, etc.)
            void Setup(void);

            // Run the game (keep the game active)
            void MainLoop(void); 

        private:
            // Main window: pointer to the GLFW window structure
            GLFWwindow *window_;

            // Sprite geometry
            Geometry *sprite_;

            // Particle geometry
            Geometry *particles_;
            Geometry *particles2_;
            Geometry* particles3_;


            // Shader for rendering sprites in the scene
            Shader sprite_shader_;

            // Shader for rendering particles
            Shader particle_shader_;
            Shader particle_shader2_;


            // References to textures
#define NUM_TEXTURES 20
            GLuint tex_[NUM_TEXTURES];

            // List of game objects
            std::vector<GameObject*> game_objects_;

            // Keep track of time
            double current_time_;

            //It's probably easier to have a seperate function for collision
            bool RayCollision(glm::vec3 start, glm::vec3 direction, glm::vec3 center, float radius);

            //New function for enemy spawning over time
            void SpawnEnemies(glm::vec3 playerPos);

            //Function to randomly spawn collectibles
            void SpawnCollectibles(glm::vec3 playerPos);

            // Callback for when the window is resized
            static void ResizeCallback(GLFWwindow* window, int width, int height);

            // Set a specific texture
            void SetTexture(GLuint w, const char *fname);

            // Load all textures
            void SetAllTextures();

            // Handle user input
            void Controls(double delta_time);

            // Update the game based on user input and simulation
            void Update(glm::mat4 view_matrix, double delta_time);

    }; // class Game

} // namespace game

#endif // GAME_H_
