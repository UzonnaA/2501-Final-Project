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
    type_ = "N/A";
    //mustDie is a bool that I turn on if the object needs to disappear automatically
    //Ex: Explosions, Bullets
    //isDead is a bool I turn on when I want to actually remove the object from the array
    isDead_ = false;
    mustDie_ = false;
    current_time_ = std::chrono::system_clock::now();
    //I could change SetMustDie to accept an int and always change mustDie to true
    //Doesn't mean I will but I totally could
    death_time_ = current_time_ + std::chrono::seconds(5);
    parent_ = nullptr;
    isChild_ = false;
    

   
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

    // Update object position with Euler integration
    position_ += velocity_ * ((float) delta_time);
    current_time_ = std::chrono::system_clock::now();
    
    //I constantly update the time and if the conditions are true, "kill" the object
    if (mustDie_ && current_time_ > death_time_) {
        isDead_ = true;
        //std::cout << "A GameObject has perished" << std::endl;
    }

    if (isChild_) {
        if (parent_->CheckDead()) {
            isDead_ = true;
        }
    }

    if (type_ == "blade") {
        angle_ = angle_ + ((glm::pi<float>() / 500.0f) * (delta_time*900.0));
    }

  
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

    // Bind the entity's texture
    glBindTexture(GL_TEXTURE_2D, texture_);

    // Draw the entity
    glDrawElements(GL_TRIANGLES, geometry_->GetSize(), GL_UNSIGNED_INT, 0);
}

} // namespace game