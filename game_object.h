#ifndef GAME_OBJECT_H_
#define GAME_OBJECT_H_

#include <glm/glm.hpp>
#define GLEW_STATIC
#include <GL/glew.h>

#include <chrono>
#include <thread>
#include <string>

#include "shader.h"
#include "geometry.h"

namespace game {

    /*
        GameObject is responsible for handling the rendering and updating of one object in the game world
        The update and render methods are virtual, so you can inherit them from GameObject and override the update or render functionality (see PlayerGameObject for reference)
    */


    class GameObject {

        public:
            // Constructor
            GameObject(const glm::vec3 &position, Geometry *geom, Shader *shader, GLuint texture);

            // Update the GameObject's state. Can be overriden in children
            virtual void Update(double delta_time);

            // Renders the GameObject 
            virtual void Render(glm::mat4 view_matrix, double current_time);

            // Getters
            inline glm::vec3& GetPosition(void) { return position_; }
            inline float GetScale(void) { return scale_; }
            inline float GetAngle(void) { return angle_; }
            inline glm::vec3& GetVelocity(void) { return velocity_; }
            inline bool CheckDead(void) { return isDead_; }
            inline bool CheckMustDie(void) { return mustDie_; }
            inline bool CheckIfChild(void) { return isChild_; }
            inline std::string GetType(void) { return type_; }

            // Get bearing direction (direction in which the game object
            // is facing)
            glm::vec3 GetBearing(void);

            // Get vector pointing to the right side of the game object
            glm::vec3 GetRight(void);

            // Setters
            inline void SetPosition(const glm::vec3& position) {
                if (!isChild_) {
                    position_ = position;
                }
                else {
                    position_ = parent_->GetPosition() + position;
                }
            }


            inline void SetScale(float scale) { scale_ = scale; }
            void SetAngle(float angle);
            inline void SetVelocity(const glm::vec3& velocity) { velocity_ = velocity; }
            inline void SetMustDie(bool die) { mustDie_ = die; }
            inline void SetType(std::string type) { type_ = type; }
            
            inline void SetParent(GameObject* parent) { 
                parent_ = parent; 
                isChild_ = true;
            }

            inline void Kill() { isDead_ = true; }

        protected:
            // Object's Transform Variables
            glm::vec3 position_;
            float scale_;
            float angle_;
            glm::vec3 velocity_;
            std::string type_;

            // Geometry
            Geometry *geometry_;
 
            // Shader
            Shader *shader_;

            // Object's texture reference
            GLuint texture_;

            //This will check if the GO should be destroyed
            bool isDead_;
            bool mustDie_;

           

            //These will track when to kill the object
            std::chrono::time_point<std::chrono::system_clock> current_time_, death_time_;

            //Need this to do hierarchy transforms
            GameObject* parent_;
            bool isChild_;

            

    }; // class GameObject

} // namespace game

#endif // GAME_OBJECT_H_