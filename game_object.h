#ifndef GAME_OBJECT_H_
#define GAME_OBJECT_H_

#include <glm/glm.hpp>
#define GLEW_STATIC
#include <GL/glew.h>

#include <chrono>
#include <thread>
#include <string>
#include <iostream>
#include <vector>

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
            void LookAtPlayer();

            void InitFiring(Geometry* geom, Shader* shader, GLuint texture, std::vector<GameObject*>& vec, int type);
            void Fire();

            // Getters
            inline glm::vec3& GetPosition(void) { return position_; }
            inline float GetScale(void) { return scale_; }
            inline int GetKillCount(void) { return killCount_; }
            inline int GetStarCount(void) { return stars_collected_; }
            inline int GetHealth(void) { return health_; }
            inline float GetAngle(void) { return angle_; }
            inline float GetRadius(void) { return radius_; }
            inline int GetWeaponType(void) { return weaponType_; }
            inline glm::vec3& GetVelocity(void) { return velocity_; }
            inline bool CheckDead(void) { return isDead_; }
            inline bool CheckMustDie(void) { return mustDie_; }
            inline bool CheckGhost(void) { return ghost_; }
            inline bool CheckIfChild(void) { return isChild_; }
            inline std::string GetType(void) { return type_; }
            inline bool isBackground(void) { return isBg_; }
            // Get bearing direction (direction in which the game object
            // is facing)
            glm::vec3 GetBearing(void);

            // Get vector pointing to the right side of the game object
            glm::vec3 GetRight(void);

            // Setters
            inline void SetPosition(const glm::vec3& position) {
                glm::vec3 newPos = position;
                
                
                if (type_ == "player") {
                    if (newPos.x < -4.2f) {
                        newPos.x = 4.2f;
                    }
                    if (newPos.x > 4.2f) {
                        newPos.x = -4.2f;
                    }
                }
                
                
                if (!isChild_) {
                    position_ = newPos;
                }
                else {
                    position_ = parent_->GetPosition() + newPos;
                }
            }

            inline void IncrementWeaponType() {
                if (weaponType_ < 3) { 
                    this->weaponType_++;
                }
                else if (weaponType_ >= 3) {
                    this->weaponType_ = 1; //back to default weapon (which is bullet)
                }
            }

            inline void SetScale(float scale) { scale_ = scale; }
            void SetAngle(float angle);
            inline void SetVelocity(const glm::vec3& velocity) { 
                glm::vec3 newVel = velocity;

                if (type_ == "player") {
                    if (newVel.x < -3.0f) {
                        newVel.x = -3.0;
                    }
                    if (newVel.x > 3.0f) {
                        newVel.x = 3.0f;
                    }
                }
                velocity_ = newVel; 
            }
            
            //If you are turnig off must die for some weird reason
            //the value of time doesn't matter
            inline void SetMustDie(bool die, int time) { 
                mustDie_ = die;
                current_time_ = std::chrono::system_clock::now();
                death_time_ = current_time_ + std::chrono::seconds(time);
            }

            inline void IncrementKillCount(void) { killCount_ += 1; }
            inline void SetIsBg(bool isBg) { isBg_ = isBg; }
            inline void SetGhost(bool ghost) { ghost_ = ghost; }
            
            
            inline void SetType(std::string type) { 
                type_ = type;
                if (type_ == "enemy") {
                    //change the health do a different value perhaps?
                    //Don't have to, but you could do that here
                }
            }
            
            inline void TakeDamage(int damage) { health_ -= damage; }
            inline void SetHealth(int health) {health_ = health;}
            inline void SetStarCount(int stars) { stars_collected_ = stars; }

            inline void SetParent(GameObject* parent) { 
                parent_ = parent; 
                isChild_ = true;
            }

            inline void Kill() { isDead_ = true; }

            //for enemy
            //Getters
            inline int GetMovement(void) { return state_; }
            //Setter
            inline void SetMovement(int state) { state_ = state; }
            inline void SetPlayer(glm::vec3 player) { player_pos_ = player; }
            inline void SetGoldShip(GLuint gold) { gold_texture_ = gold; }
            void UpdateEnemy(double delta_time);

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
            GLuint gold_texture_;


            //This will check if the GO should be destroyed
            bool isDead_;
            bool mustDie_;

            //The enemy also has bullets and this will allow it to fire
            bool enemyCanFire;
            Geometry* geometryBullet_;
            Shader* shaderBullet_;
            GLuint textureBullet_;
            std::vector<GameObject*>* MainVector;
            

           

            //These will track when to kill the object
            std::chrono::time_point<std::chrono::system_clock> current_time_, death_time_, invincible_time_;
            
            std::chrono::time_point<std::chrono::system_clock> fire_time_;

            //Need this to do hierarchy transforms
            GameObject* parent_;
            bool isChild_;

            //Keep track of player kills
            int killCount_;

            //Keep track of health
            int health_;

            //Keep track of starts collected for ghosting
            int stars_collected_;

            //for enemy
            int state_;
            float radius_;
            float speed_;
            glm::vec3 centre_;
            glm::vec3 player_pos_;

            //for weapon
            int weaponType_;

            //for bg
            bool isBg_;

            //This will allow for a GameObject to become invincible
            //I'm using this for enemies after they explode, but it will also work for the player
            bool ghost_;

            //using this so the enemy minigun fires in bursts
            int burst_;

            

    }; // class GameObject

} // namespace game

#endif // GAME_OBJECT_H_
