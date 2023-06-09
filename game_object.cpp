#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "game_object.h"

namespace game {


GameObject::GameObject(const glm::vec3 &position, Geometry *geom, Shader *shader, GLuint texture) 
{

    // Initialize all attributes
    position_ = position;
    scale_ = 1.0;
    angle_ = 0.0;
    velocity_ = glm::vec3(0.0f, 0.0f, 0.0f); // Starts out stationary
    geometry_ = geom;
    shader_ = shader;
    texture_ = texture;
    gold_texture_ = texture;
    type_ = "N/A";
    //mustDie is a bool that I turn on if the object needs to disappear automatically
    //Ex: Explosions, Bullets
    //isDead is a bool I turn on when I want to actually remove the object from the array
    isDead_ = false;
    isBg_ = false;  //to distinguish background easily
    mustDie_ = false;
    current_time_ = std::chrono::system_clock::now();
    
    //I set this to the really high value of 100, but in reality
    //the code always forces you to change it anyway
    //No, this does not mean everything will delete after 100s
    //read the code
    death_time_ = current_time_ + std::chrono::seconds(100);
    fire_time_ = current_time_ + std::chrono::seconds(2);
    invincible_time_ = current_time_ + std::chrono::seconds(100);
    parent_ = nullptr;
    isChild_ = false;
    killCount_ = 0;
    //feel free to change this value
    health_ = 5;

    //for enemy
    radius_ = 1.0f;
    speed_ = 0.2f;
    centre_ = position_;
    player_pos_ = glm::vec3(0.0f, 1.0f, 0.0f);
    angle_ = 0.0f;

    //for weapon
    weaponType_ = 1;
    burst_ = 0;

    //for invincibility
    ghost_ = false;
    stars_collected_ = 0;

    //For enemy bullets
    enemyCanFire = false;
    geometryBullet_ = geom;
    shaderBullet_ = shader;
    textureBullet_ = texture;
    MainVector = nullptr;


    
   
}




glm::vec3 GameObject::GetBearing(void) {

    // Assumes sprite is initially rotated by 90 degrees
    float pi_over_two = glm::pi<float>() / 2.0f;
    glm::vec3 dir(cos(angle_ + pi_over_two), sin(angle_ + pi_over_two), 0.0);
    return dir;
}


glm::vec3 GameObject::GetRight(void) {

    // Assumes sprite is initially rotated by 90 degrees
    glm::vec3 dir(cos(angle_), sin(angle_), 0.0);
    return dir;
}


void GameObject::SetAngle(float angle){ 

    // Set angle of the game object
    // Make sure angle is in the range [0, 2*pi]
    float two_pi = 2.0f*glm::pi<float>();
    angle = fmod(angle, two_pi);
    if (angle < 0.0){
        angle += two_pi;
    }
    angle_ = angle;
}




void GameObject::Update(double delta_time) {

    if (this->GetType() == "enemy") {
        //Calculate the distance from enemy to player
        float distance = glm::distance(position_, player_pos_);
        const float attack_range = 2.0f;

        //If they are too close, attack them
        //Otherwise patrol
        if (distance <= attack_range) {
            state_ = 1;
        }
        else {
            state_ = 0;
        }

        //State 0 is patrol
        //Uses a parametric equation to move in a slow circle
        if (state_ == 0) {
            angle_ += speed_ * delta_time;
            float x = centre_.x + (radius_ * cos(angle_));
            float y = centre_.y + (radius_ * sin(angle_));

            position_ = glm::vec3(x, y, 0.0f);
        }

        //State 1 is move
        //Moves in the direction of the given player position
        if (state_ == 1) {
            glm::vec3 direction = player_pos_ - position_;
            direction = glm::normalize(direction);
            position_ += glm::vec3(direction.x * 0.5f * delta_time, direction.y * 0.5f * delta_time, 0.0f);
            LookAtPlayer();
        }

        if (state_ == 2) {

        }

        if (position_.y < player_pos_.y - 2) {
            Kill();
            std::cout << "Killed offsceen" << std::endl;
        }
        
    }
    // Update object position with Euler integration
    position_ += velocity_ * ((float) delta_time);
    current_time_ = std::chrono::system_clock::now();
    
    //I constantly update the time and if the conditions are true, "kill" the object
    if (mustDie_ && current_time_ > death_time_) {
        isDead_ = true;
        //std::cout << "A GameObject has perished" << std::endl;
    }

    if (!isDead_ && current_time_ > fire_time_) {
            Fire();  
    }

    if (isChild_) {
        if (parent_->CheckDead()) {
            isDead_ = true;
        }
    }

    if (type_ == "blade") {
        angle_ = angle_ + ((glm::pi<float>() / 500.0f) * (delta_time*900.0));
    }

    //Logic for ghost mode
    if (type_ == "player" && stars_collected_ == 1 && !ghost_) {
        SetGhost(true);
        stars_collected_ = 0;
        invincible_time_ = current_time_ + std::chrono::seconds(5);
    }

    if (type_ == "player" && ghost_ && current_time_ > invincible_time_) {
        SetGhost(false);
        stars_collected_ = 0;
    }


  
}
/*
HOW TO MAKE ENEMIES FIRE DIFFERENT BULLETS

1. Make a new enemy sprite, we'll call this enemy2. Uzonna will handle how to decide when new enemies spawn
2. Call enemy2.InitFiring() in game.cpp and insert the values needed for a new bullet (like a different texture and stuff, you'll see an example is already there)
NOTE: Enemies will only hold one kind of bullet data, so when we init we decide the bullet
3. Add if statements to the Fire() function below to fire the correct bullet based on weaponType, example shown
5. Make sure to change the fire rate by editing the fire_time. Remember enemies should only fire every 1-5s seconds, unlike the player
6. DONE!



*/

void GameObject::InitFiring(Geometry* geom, Shader* shader, GLuint texture, std::vector<GameObject*>& vec, int type) {
    //This code grants the gameobject the needed data to fire a bullet
    geometryBullet_ = geom;
    shaderBullet_ = shader;
    textureBullet_ = texture;
    MainVector = &vec;
    weaponType_ = type;
    enemyCanFire = true;

}

void GameObject::Fire() {
    //Here decides what bullet the gameobject will fire
    //Different weapon types and such would be decided here

    //1 is normal bullet
    //2 is aoe
    //3 is minigun

    //I'll probably change spawn rates later too (maybe 60/30/10?)
    if (enemyCanFire && weaponType_ == 1) {
        GameObject* bullet = new GameObject(GetPosition(), geometryBullet_, shaderBullet_, textureBullet_);


        bullet->SetScale(0.5);
        bullet->SetMustDie(true, 15);
        bullet->SetAngle(GetAngle());
        bullet->SetVelocity(5.0f * GetBearing());
        bullet->SetType("enemyBullet");
        MainVector->insert(MainVector->begin() + 1, bullet);
        fire_time_ = current_time_ + std::chrono::seconds(2);
    }

    if (enemyCanFire && weaponType_ == 2) {
        GameObject* bullet = new GameObject(GetPosition(), geometryBullet_, shaderBullet_, textureBullet_);


        bullet->SetScale(1.5);
        bullet->SetMustDie(true, 15);
        bullet->SetAngle(GetAngle());
        bullet->SetVelocity(2.5f * GetBearing());
        bullet->SetType("enemyBullet");
        MainVector->insert(MainVector->begin() + 1, bullet);
        fire_time_ = current_time_ + std::chrono::seconds(4);



    }

    if (enemyCanFire && weaponType_ == 3) {
        GameObject* bullet = new GameObject(GetPosition(), geometryBullet_, shaderBullet_, textureBullet_);


        bullet->SetScale(0.15);
        bullet->SetMustDie(true, 15);
        bullet->SetAngle(GetAngle());
        bullet->SetVelocity(5.0f * GetBearing());
        bullet->SetType("enemyBullet");
        MainVector->insert(MainVector->begin() + 1, bullet);
        burst_ += 1;

        if (burst_ == 3) {
            fire_time_ = current_time_ + std::chrono::seconds(2);
            burst_ = 0;
        }
        else {
            fire_time_ = current_time_ + std::chrono::milliseconds(200);
        }


        



        

    }

}


void GameObject::LookAtPlayer() {
    //handles enemies turning at the player
    //Changes their dir based on playerpos to keep it brief
    glm::vec3 direction = player_pos_ - position_;
    float angle = atan2(direction.y, direction.x);
    angle -= glm::half_pi<float>();
    SetAngle(angle);
}


void GameObject::Render(glm::mat4 view_matrix, double current_time){

    // Set up the shader
    shader_->Enable();

    // Set up the view matrix
    shader_->SetUniformMat4("view_matrix", view_matrix);

    // Setup the scaling matrix for the shader
    glm::mat4 scaling_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale_, scale_, 1.0));

    // Setup the rotation matrix for the shader
    glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), angle_, glm::vec3(0.0, 0.0, 1.0));

    // Set up the translation matrix for the shader
    glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), position_);

    // Setup the transformation matrix for the shader
    glm::mat4 transformation_matrix = translation_matrix * rotation_matrix * scaling_matrix;

    // Set the transformation matrix in the shader
    shader_->SetUniformMat4("transformation_matrix", transformation_matrix);

    // Set up the geometry
    geometry_->SetGeometry(shader_->GetShaderProgram());

    if (this->isBg_ == true) {
        shader_->SetUniform1f("x", 140.0);   //when it is a background the x value goes up so that the background doesnt look stretched and values are properly interpolated      
    }
    else {
        shader_->SetUniform1f("x", 1.0);
    }

    if (ghost_ && type_ == "player") {
        glBindTexture(GL_TEXTURE_2D, gold_texture_);
    }
    else {
        glBindTexture(GL_TEXTURE_2D, texture_);
    }
    

    

    // Draw the entity
    glDrawElements(GL_TRIANGLES, geometry_->GetSize(), GL_UNSIGNED_INT, 0);
}

} // namespace game
