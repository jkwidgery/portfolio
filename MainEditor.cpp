/*****************************************************************//**
 * \file   MainEditor.cpp
 * \brief  contains main editor implementation
 *
 * \author Jasmine Widgery, Emma Lewarne
 * \date   September 2021
 *
 * \param Copyright © 2021 DigiPen (USA) Corporation.
 *********************************************************************/

#include "ImGUI/imgui.h"
#include "ImGUI/imconfig.h"
#include "MainEditor.h"
#include <vector>
#include <iostream>
#include <fstream>
#include "GameObjectManager.h"
#include "GameObject.h"
#include "glm/glm.hpp"
#include "EditorSystem.h"
#include "rttr/registration.h"
#include "rttr/array_range.h"
#include <tgmath.h>
#include "Camera.h"
#include "Mesh.h"
#include "FMODStudioCore.h"
#include <unordered_map>
#include <string>
#include <algorithm>
#include "Collider.h"
#include "InputController.h"
#include "AudioEditor.h"
#include "Engine.h"
#include "ArtEditor.h"
#include "Debug.h"
#include "SpriteManager.h"
#include "PrefabManager.h"
#include "Messaging.h"
#include "SceneManager.h"
#include "RigidBody.h"
#include "Behavior.h"
#include "Graphics.h"
#include "SpineAnimationManager.h"
#include "TextureManager.h"
#include "SpriteAnimationManager.h"
#include "Particles.h"
#include "UI.h"
#include "Scene.h"
#include "Button.h"
#include "BossAnimationComponent.h"
#include "PostProcessing.h"
#include "AnimationLooper.h"
#undef GetObject

extern bool editor_enabled;
namespace CloudEngine
{
	//redirect console output from std console to in-editor console
	static std::stringstream buffer;
	std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

	//create instance of directory for file explorer
	extern class Directory;
	Directory dir;

	namespace Editor
	{
		static MenuBar* mbInstance_;
	}
	//Currently selected file from Editor directory
	ObjectManagement::FileObject* activeFile;

	//Currently selected component from inspector
	Components::Component* globalComp = nullptr;

	//global instance of file options window
	Editor::FileOptionsWindow* fso;

	//global instance of sprite viewer
	Editor::SpineAnimationViewer* spriteViewer;

	/**
	 * This is the constructor for the menu bar.
	 */
	Editor::MenuBar::MenuBar()
	{
		//Object editing labels
		labelresize_ = "Resize";
		labelrotate_ = "Rotate";
		labeltranslate_ = "Translate";
		
		//initialize values
		objToMouseCurr.x = 0;
		objToMousePrev.x = 0;
		objToMouseCurr.y = 0;
		objToMousePrev.y = 0;
		oldRotate_ = 0;
		oldTranslation_.x = 0;
		oldTranslation_.y = 0;
		oldTranslation_.z = 0;
		oldScale_.x = 0;
		oldScale_.y = 0;
		oldScale_.z = 0;

		//create instance
		mbInstance_ = this;
	}
	/**
	* draws console window
	*/
	void CloudEngine::Editor::ConsoleWindow::Draw()
	{
		ImGui::TextUnformatted(buffer.str().c_str());
	}

	/**
	* This function looks up a component in a game object
	* \param obj
	* \return bool
	*
	*/
	bool ComponentLookup(rttr::type component, CloudEngine::Components::GameObject* obj)
	{
		std::vector< CloudEngine::Components::Component*> objComponents = obj->GetComponents();
		for (int i = 0; i < objComponents.size(); i++)
		{
			if (component == objComponents[i]->get_type())
			{
				return true;
			}
		}
		return false;
	}

	/**
	 * This function removes a component through the editor
	 * \param obj
	 * \return bool
	 */
	bool removeComponent(rttr::type component, CloudEngine::Components::GameObject* obj)
	{
		std::vector< CloudEngine::Components::Component*> objComponents = obj->GetComponents();
		for (unsigned i = 0; i < objComponents.size(); i++)
		{
			if (component == objComponents[i]->get_type())
			{
				obj->RemoveComponent(objComponents[i]);
				return true;
			}
		}
		return false;
	}

	/**
	 * This function adds a component through the editor
	 * \param obj
	 * \return bool
	 */
	bool addComponent(rttr::type component, CloudEngine::Components::GameObject* obj)
	{
		obj->AddComponent(component);
		return true;
	}

	//Helper struct for ImGui stuff
	struct InputTextCallback_UserData
	{
		std::string* Str;
		ImGuiInputTextCallback  ChainCallback;
		void* ChainCallbackUserData;
	};


	//initialize menu bar things
	//unsure why I put it outside of the constructor
	//but I'm scared to move it
	bool Editor::MenuBar::resizeBtn_ = false;
	bool Editor::MenuBar::translateBtn_ = false;
	bool Editor::MenuBar::rotateBtn_ = false;

	bool Editor::MenuBar::audioMode_ = false;
	bool Editor::MenuBar::mainMode_ = false;
	bool Editor::MenuBar::artMode_ = false;

	/**
	 * returns singleton instance of menu bar
	 * \return MenuBar*
	 */
	Editor::MenuBar* Editor::MenuBar::GetInstance()
	{
		return mbInstance_ ? mbInstance_ : mbInstance_ = new MenuBar;
	}


	/**
	* Initializes instance
	*/
	void Editor::MenuBar::Init()
	{
		mbInstance_ = this;
	}

	/**
	* Determines if object is copied
	*/
	void Editor::SceneHierarchyWindow::isCopied()
	{
		Components::GameObject* selected = GetSelectedObject();
		if (selected)
		{
			if (InputController::keyCodePressed(SDLK_LCTRL) || InputController::keyCodePressed(SDLK_RCTRL))
			{
				if (InputController::keyCodeTriggered(SDLK_c))
				{
					copiedObj_ = selected;
				}
			}
		}
	}

	/**
	* Determines if object is pasted
	*/
	void Editor::SceneHierarchyWindow::isPasted()
	{
		if (copiedObj_)
		{
			if (InputController::keyCodePressed(SDLK_LCTRL) || InputController::keyCodePressed(SDLK_RCTRL))
			{
				if (InputController::keyCodeTriggered(SDLK_v))
				{
					Components::GameObject* clone = copiedObj_->Clone();
					Components::Transform* cloneTransform = clone->GetComponent<Components::Transform>();
					glm::vec2 worldPos = Graphics::Camera::GetActiveCamera()->GetScreenToWorldMatrix() * glm::vec4(Engine::Core::WindowWidth() / 2, Engine::Core::WindowHeight() / 2, 0.0f, 1.0f);
					cloneTransform->SetTranslation(glm::vec3(worldPos, 0.0f));
					Components::AddObjectInManager(clone, Components::M_ACTIVE_LIST);
				}

			}

		}

	}

	/**
	 * does math to find which object the user has clicked
	 */
	void Editor::MenuBar::getSelectedObject()
	{
		// Catch any cases where we don't want to change the selected object
		if (translateBtn_) return;
		if (rotateBtn_) return;
		if (resizeBtn_) return;
		if (ImGui::GetIO().WantCaptureMouse) return;
		if (!InputController::leftMouseButtonUp()) return;

		using namespace Components;
		using namespace Graphics;
		using namespace std;
		using namespace glm;

		Camera* active = Graphics::Camera::GetActiveCamera();
		if (active == NULL) return;

		// all clicked objects
		vector<GameObject*>& objList = CloudEngine::Components::GetActiveList();
		map<float, vector<GameObject*>> clickedObjects;

		vec2 worldMousePos = active->GetScreenToWorldMatrix() * glm::vec4(InputController::getMousePos(), 0, 1);
		vec2 worldCameraPos = active->Position();

		for (Components::GameObject* go : objList)
		{
			Transform* trans = go->GetComponent<Transform>();
			glm::vec3 position = *trans->GetTranslation();
			glm::vec3 scale = *trans->GetScale();
			

			if ((worldMousePos.x < position.x + scale.x) && (worldMousePos.x > position.x - scale.x)
				&& (worldMousePos.y > position.y - scale.y) && (worldMousePos.y < position.y + scale.y))
			{
				clickedObjects[position.z].push_back(go);
			}
		}

		if (clickedObjects.size() == 0) { SetSelectedObject(NULL); return; }

		// closest objects to camera
		vector<GameObject*>& selectedPlane = (clickedObjects.begin())->second;

		// closest object to mouse
		float closestDistance = std::numeric_limits<float>::max();
		GameObject* closest = NULL;

		for (int i = 0; i < selectedPlane.size(); i++)
		{
			Components::GameObject* go = selectedPlane.operator[](i);
			Transform* trans = go->GetComponent<Transform>();
			glm::vec2 worldPos = (*trans->GetMatrix())[3];
			float distance;

			if ((distance = length(worldPos - worldCameraPos)) < closestDistance)
			{
				closestDistance = distance;
				closest = go;
			}
		}

		SetSelectedObject(closest);
	}

	static Components::GameObject* selected = NULL;

	/**
	 * Gets the selected object.
	 * 
	 * \return GameObject*
	 */
	Components::GameObject* Editor::GetSelectedObject()
	{
		return selected;
	}

	/**
	 * Set the selected object.
	 *
	 * \param obj
	 */
	void Editor::SetSelectedObject(Components::GameObject* obj)
	{
		selected = obj;
	}

	/**
	 * This function breaks down types passed into it so that they can be displayed by ImGui.
	 *
	 * \param instance - rttr::variant property to be broken down
	 * \param name - name of property to be broken down
	 *
	 */
	void Editor::getBaseType(rttr::variant& instance, std::string name)
	{
		if (!instance.is_valid())
		{
			printf("Not a valid instance %s \n", name.c_str());
			return;
		}

		if (instance.is_type<float>())
		{
			float& v = instance.get_value<float>();
			ImGui::InputFloat(name.c_str(), &v);
		}
		else if (instance.is_type<Graphics::GraphicsSystem::BlendMode>())
		{
			Graphics::GraphicsSystem::BlendMode& blendMode = instance.get_value<Graphics::GraphicsSystem::BlendMode>();

			const static char* blendModeNames[] = {
				"Alpha",
				"Multiply",
				"Add",
				"Subtract",
				"Colour Dodge"
			};

			if (ImGui::BeginCombo(name.c_str(), blendModeNames[(int)blendMode], ImGuiComboFlags_NoArrowButton))
			{
				for (int i = 0; i < 5; i++)
				{
					if (ImGui::Selectable(blendModeNames[i], i == (int)blendMode))
					{
						blendMode = (Graphics::GraphicsSystem::BlendMode)i;
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}
		}
		else if (instance.is_type<Graphics::GraphicsSystem::TintBlendMode>())
		{
			Graphics::GraphicsSystem::TintBlendMode& blendMode = instance.get_value<Graphics::GraphicsSystem::TintBlendMode>();

			const static char* tintBlendModeNames[] = {
				"Multiply",
				"Add",
				"Subtract",
				"Interpolate"
			};

			if (ImGui::BeginCombo(name.c_str(), tintBlendModeNames[(int)blendMode], ImGuiComboFlags_NoArrowButton))
			{
				for (int i = 0; i < 4; i++)
				{
					if (ImGui::Selectable(tintBlendModeNames[i], i == (int)blendMode))
					{
						blendMode = (Graphics::GraphicsSystem::TintBlendMode)i;
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndCombo();
			}
		}
		else if (instance.is_type<double>())
		{
			double& v = instance.get_value<double>();
			ImGui::InputDouble(name.c_str(), &v);
		}
		else if (instance.is_type<int>())
		{
			int& v = instance.get_value<int>();
			ImGui::InputInt(name.c_str(), &v);
		}
		else if (instance.is_type<bool>())
		{
			bool& v = instance.get_value<bool>();
			ImGui::Checkbox(name.c_str(), &v);
		}
		else if (instance.is_type<std::string>())
		{
			std::string& v = instance.get_value<std::string>();

			Editor::InputText(name.c_str(), &v);
		}
		else if (instance.is_type<char*>())
		{
			char*& v = instance.get_value<char*>();
			ImGui::InputText(name.c_str(), &v[0], 256);
		}
		else if (instance.is_type<char>())
		{
			char& v = instance.get_value<char>();
			ImGui::InputInt(name.c_str(), (int*)&v);
		}
		else if (instance.is_type<unsigned int>())
		{
			int& v = instance.get_value<int>();
			ImGui::InputInt(name.c_str(), &v);
		}
		else if (instance.is_type<unsigned char>())//unsigned char
		{
			unsigned char& v = instance.get_value<unsigned char>();
			ImGui::InputScalar(name.c_str(), ImGuiDataType_S8, &v);
		}
		else if (instance.is_type<long>())
		{
			long& v = instance.get_value<long>();

		}
		else if (instance.is_type<unsigned long>())
		{
			unsigned long& v = instance.get_value<unsigned long>();
			//handle unsigned long
		}
		else if (instance.is_type<short>())
		{
			short& v = instance.get_value<short>();
			//handle short
		}
		else if (instance.is_type<unsigned short>())
		{
			unsigned short& v = instance.get_value<unsigned short>();
			//handle unsigned short
		}
		else if (instance.is_type<glm::vec3>())
		{
			ImGui::InputFloat3(name.c_str(), &(instance.get_value<glm::vec3>().x));
		}
		else if (instance.is_type<glm::vec2>())
		{
			ImGui::InputFloat2(name.c_str(), &(instance.get_value<glm::vec2>().x));
		}
		else if (instance.is_type<glm::vec4>())
		{
			ImGui::InputFloat4(name.c_str(), &(instance.get_value<glm::vec4>().x));
		}
		else if (instance.is_type<std::vector<int>>())
		{
			int& v = instance.get_value<std::vector<int>>().front();
			ImGui::InputInt(name.c_str(), &v);
		}
		else if (instance.is_type<std::vector<float>>())
		{
			float& v = instance.get_value<std::vector<float>>().front();
			ImGui::InputFloat(name.c_str(), &v);
		}
		else if (instance.is_type<std::vector<double>>())
		{
			double& v = instance.get_value<std::vector<double>>().front();
			ImGui::InputDouble(name.c_str(), &v);
		}
		else if (instance.is_type<std::vector<std::string>>())
		{
			std::string& v = instance.get_value<std::string>();
			Editor::InputText(name.c_str(), &v);
		}
		else if (instance.is_type<Graphics::Texture*>())
		{
			Graphics::Texture*& t = instance.get_value<Graphics::Texture*>();

			if (ImGui::BeginCombo(name.c_str(), t != NULL ? t->GetName().c_str() : "NONE", ImGuiComboFlags_NoArrowButton))
			{
				std::vector<ObjectManagement::Object*>& textures = ObjectManagement::TextureManager::GetInstance()->GetList();

				if (ImGui::Selectable("NONE", t == NULL)) t = NULL;
				if (t == NULL) ImGui::SetItemDefaultFocus();

				for (unsigned n = 0; n < textures.size(); n++)
				{
					bool is_selected = (t == textures[n]);
					if (ImGui::Selectable(textures[n]->GetName().c_str(), is_selected))
					{
						t = (Graphics::Texture*)textures[n];
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
		}
		else if (instance.is_type<Graphics::AnimationProxy>())
		{
			Graphics::AnimationProxy& proxy = instance.get_value<Graphics::AnimationProxy>();

			std::string c = (proxy.type > 0) ? (proxy.type == 0 ? (proxy.spriteAnimation->GetName()) : (proxy.spineAnimation->GetName())) : ("NONE");

			if (ImGui::BeginCombo(name.c_str(), c.c_str(), ImGuiComboFlags_NoArrowButton))
			{
				std::vector<ObjectManagement::Object*>& spineAnimations = ObjectManagement::SpineAnimationManager::GetInstance()->GetList();
				std::vector<ObjectManagement::Object*>& spriteAnimations = ObjectManagement::SpriteAnimationManager::GetInstance()->GetList();

				ImGui::Selectable("NONE", c == "NONE");
				if (c == "NONE")
				{
					ImGui::SetItemDefaultFocus();
				}

				for (unsigned n = 0; n < spineAnimations.size(); n++)
				{
					bool is_selected = (c == spineAnimations[n]->GetName());
					if (ImGui::Selectable(spineAnimations[n]->GetName().c_str(), is_selected))
					{
						proxy = (Graphics::SpineAnimationAsset*)spineAnimations[n];
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				for (unsigned n = 0; n < spriteAnimations.size(); n++)
				{
					bool is_selected = (c == spriteAnimations[n]->GetName());
					if (ImGui::Selectable(spriteAnimations[n]->GetName().c_str(), is_selected))
					{
						proxy = (Graphics::SpriteAnimationAsset*)spriteAnimations[n];
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
		}
		else if (instance.is_type<Graphics::SpineAnimationAsset*>())
		{
			Graphics::SpineAnimationAsset*& v = instance.get_value<Graphics::SpineAnimationAsset*>();

			if (ImGui::BeginCombo(name.c_str(), v->GetName().c_str(), ImGuiComboFlags_NoArrowButton))
			{
				std::vector<ObjectManagement::Object*>& animations = ObjectManagement::SpineAnimationManager::GetInstance()->GetList();

				for (unsigned n = 0; n < animations.size(); n++)
				{
					bool is_selected = (v == animations[n]);
					if (ImGui::Selectable(animations[n]->GetName().c_str(), is_selected))
					{
						v = (Graphics::SpineAnimationAsset*)animations[n];
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
		}
		else
		{
			if (instance.is_type<Components::Component*>())
			{

				if (instance.get_value<Components::Component*>()->get_type() == rttr::type::get<Graphics::ParticleSystem>())
				{
					if (ImGui::TreeNodeEx(name.c_str()))
					{
						using namespace Graphics;

						ParticleSystem* system = instance.get_value<ParticleSystem*>();

						int MaxParticles = system->GetMaxParticles();
						ImGui::InputInt("Max Particles", &MaxParticles);
						system->SetMaxParticles(MaxParticles);

						bool AlignToVelocity = system->GetAlignToVelocity();
						ImGui::Checkbox("Align To Velocity", &AlignToVelocity);
						system->SetAlignToVelocity(AlignToVelocity);

						bool PlayOnStart = system->PlayOnStart();
						ImGui::Checkbox("Play On Start", &PlayOnStart);
						system->PlayOnStart(PlayOnStart);

						std::vector<ParticleEffect*>& effects = system->ParticleEffects();

						int EffectCount = effects.size();
						ImGui::InputInt("Effect Count", &EffectCount);
						effects.resize(EffectCount, NULL);
						int i = 0;


						for (Graphics::ParticleEffect*& effect : effects)
						{
							rttr::type effectType = rttr::type::get<ParticleEffect>();
							std::string name = "Effect##" + std::to_string(i);
							i++;

							if (ImGui::BeginCombo(name.c_str(), effect ? effect->get_type().get_name().to_string().c_str() : "NONE"))
							{
								bool selected = effect == NULL;
								if (ImGui::Selectable("NONE", &selected))
								{
									if(effect) delete effect;
									effect = NULL;

									ImGui::SetItemDefaultFocus();
								}

								for (rttr::type type : effectType.get_derived_classes())
								{
									selected = effect ? effect->get_type() == type : false;

									if (ImGui::Selectable(type.get_name().to_string().c_str(), &selected))
									{
										if (effect) delete effect;
										effect = type.create().get_value<ParticleEffect*>()->Clone();
										ImGui::SetItemDefaultFocus();
									}
								}
								ImGui::EndCombo();
							}

							if (effect != NULL)
							{
								if (ImGui::TreeNodeEx(effect->get_type().get_name().to_string().c_str()))
								{
									effect->DrawEditor();

									ImGui::TreePop();
								}
							}
						}
						ImGui::TreePop();
					}
				}
				else
				if (instance.get_value<Components::Component*>()->get_type() == rttr::type::get<UI::Button>())
				{
					if (ImGui::TreeNodeEx(name.c_str()))
					{
						using namespace UI;
						using namespace Graphics;

						Button* button = instance.get_value<Button*>();
						Animation* animation = button->Parent()->GetComponent<Animation>();

						const std::string& currentAnimation = button->GetAnimation();

						if (animation)
						{
							AnimationProxy ani = animation->GetAnimation();

							if (ani.type != -1)
							{
								if (ImGui::BeginCombo("Animation", currentAnimation.c_str(), ImGuiComboFlags_NoArrowButton))
								{
									bool selected = currentAnimation == "NONE";
									if (ImGui::Selectable("NONE", &selected))
									{
										button->SetAnimation("NONE");
										ImGui::SetItemDefaultFocus();
									}

									switch (ani.type)
									{
									case 0:
									{
										SpriteAnimationAsset* animationAsset = ani.spriteAnimation;

										for (const std::pair<std::string, SpriteAnimationSequence>& sequence : animationAsset->_Sequences())
										{
											bool selected = currentAnimation == sequence.first;

											if (ImGui::Selectable(sequence.first.c_str(), &selected))
											{
												button->SetAnimation(sequence.first);
												ImGui::SetItemDefaultFocus();
											}
										}
										break;
									}
									case 1:
									{
										SpineAnimationAsset* animationAsset = ani.spineAnimation;
										spine::Vector<spine::Animation*> sequences = animationAsset->GetAnimationStateData()->getSkeletonData()->getAnimations();

										for (int i = 0; i < sequences.size(); i ++)
										{
											bool selected = currentAnimation == sequences[i]->getName().buffer();

											if (ImGui::Selectable(sequences[i]->getName().buffer(), &selected))
											{
												button->SetAnimation(sequences[i]->getName().buffer());
												ImGui::SetItemDefaultFocus();
											}
										}
										break;
									}
									}

									ImGui::EndCombo();
								}
							}
						}
						else
						{
							button->SetAnimation("NONE");
						}

						int v = button->GetButtonID();
						ImGui::InputInt("Button ID", &v);
						button->SetButtonID(v);

						bool b = button->GetBackButton();
						ImGui::Checkbox("Back Button", &b);
						button->SetBackButton(b);

						bool g = button->GetUnpauseButton();
						ImGui::Checkbox("Pause Button", &g);
						button->SetUnpauseButton(g);

						ImGui::TreePop();
					}
				}
				else
				if (instance.get_value<Components::Component*>()->get_type() == rttr::type::get<Graphics::Renderer>())
				{
					if (ImGui::TreeNodeEx(name.c_str()))
					{
						using namespace Graphics;

						Renderer* renderer = instance.get_value<Renderer*>();

						Graphics::Texture*& t = renderer->texture;

						if (ImGui::BeginCombo("Texture", t != NULL ? t->GetName().c_str() : "NONE", ImGuiComboFlags_NoArrowButton))
						{
							std::vector<ObjectManagement::Object*>& textures = ObjectManagement::TextureManager::GetInstance()->GetList();

							if (ImGui::Selectable("NONE", t == NULL)) t = NULL;
							if (t == NULL) ImGui::SetItemDefaultFocus();

							for (unsigned n = 0; n < textures.size(); n++)
							{
								bool is_selected = (t == textures[n]);
								if (ImGui::Selectable(textures[n]->GetName().c_str(), is_selected))
								{
									t = (Graphics::Texture*)textures[n];
								}
								if (is_selected)
									ImGui::SetItemDefaultFocus();
							}

							ImGui::EndCombo();
						}

						ImGui::ColorPicker4("Tint", &renderer->tint.r, ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
						ImGui::InputFloat("Opacity", &renderer->opacity);
						
						rttr::variant v = renderer->blending;
						getBaseType(v, "Blend Mode");
						renderer->blending = v.get_value<Graphics::GraphicsSystem::BlendMode>();

						v = renderer->tintBlending;
						getBaseType(v, "Tint Blend Mode");
						renderer->tintBlending = v.get_value<Graphics::GraphicsSystem::TintBlendMode>();

						ImGui::Checkbox("Flip X", &renderer->flipX);
						ImGui::Checkbox("Flip Y", &renderer->flipY);
						ImGui::Checkbox("UI", &renderer->UI);
						ImGui::Checkbox("Parallax", &renderer->parallax);

						Graphics::Texture*& e = renderer->emissionTexture;

						if (ImGui::BeginCombo("Emission", e != NULL ? e->GetName().c_str() : "NONE", ImGuiComboFlags_NoArrowButton))
						{
							std::vector<ObjectManagement::Object*>& textures = ObjectManagement::TextureManager::GetInstance()->GetList();

							if (ImGui::Selectable("NONE", e == NULL)) e = NULL;
							if (e == NULL) ImGui::SetItemDefaultFocus();

							for (unsigned n = 0; n < textures.size(); n++)
							{
								bool is_selected = (e == textures[n]);
								if (ImGui::Selectable(textures[n]->GetName().c_str(), is_selected))
								{
									e = (Graphics::Texture*)textures[n];
								}
								if (is_selected)
									ImGui::SetItemDefaultFocus();
							}

							ImGui::EndCombo();
						}

						ImGui::ColorPicker3("Emission Tint", &renderer->emissionTint.r, ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);
						ImGui::InputFloat("Emission Strength", &renderer->emissionStrength);

						ImGui::TreePop();
					}
				}
				else
				if (instance.get_value<Components::Component*>()->get_type() == rttr::type::get<Graphics::Animation>())
				{
					if (ImGui::TreeNodeEx(name.c_str()))
					{
						using namespace UI;
						using namespace Graphics;

						Animation* animation = instance.get_value<Animation*>();

						Graphics::AnimationProxy proxy = animation->GetAnimation();

						std::string c = (proxy.type > 0) ? (proxy.type == 0 ? (proxy.spriteAnimation ? proxy.spriteAnimation->GetName() : "NONE") : (proxy.spineAnimation ? proxy.spineAnimation->GetName() : "NONE")) : ("NONE");

						if (ImGui::BeginCombo(name.c_str(), c.c_str(), ImGuiComboFlags_NoArrowButton))
						{
							std::vector<ObjectManagement::Object*>& spineAnimations = ObjectManagement::SpineAnimationManager::GetInstance()->GetList();
							std::vector<ObjectManagement::Object*>& spriteAnimations = ObjectManagement::SpriteAnimationManager::GetInstance()->GetList();

							ImGui::Selectable("NONE", c == "NONE");
							if (c == "NONE")
							{
								ImGui::SetItemDefaultFocus();
							}

							for (unsigned n = 0; n < spineAnimations.size(); n++)
							{
								bool is_selected = (c == spineAnimations[n]->GetName());
								if (ImGui::Selectable(spineAnimations[n]->GetName().c_str(), is_selected))
								{
									proxy = (Graphics::SpineAnimationAsset*)spineAnimations[n];
									animation->SetAnimation(proxy);
								}
								if (is_selected)
									ImGui::SetItemDefaultFocus();
							}

							for (unsigned n = 0; n < spriteAnimations.size(); n++)
							{
								bool is_selected = (c == spriteAnimations[n]->GetName());
								if (ImGui::Selectable(spriteAnimations[n]->GetName().c_str(), is_selected))
								{
									proxy = (Graphics::SpriteAnimationAsset*)spriteAnimations[n];
									animation->SetAnimation(proxy);
								}
								if (is_selected)
									ImGui::SetItemDefaultFocus();
							}

							ImGui::EndCombo();
						}


						const std::string currentAnimation = animation->CurrentAnimation();

						if (0)
						{
							ImGui::TextWrapped("Warning, this may not work correctly if an attached behaviour uses animations:");
							if (ImGui::BeginCombo("Preview Sequence", currentAnimation.c_str(), ImGuiComboFlags_NoArrowButton))
							{
								bool selected = currentAnimation == "NONE";
								if (ImGui::Selectable("NONE", &selected))
								{
									animation->PlayAnimationLooped("NONE");
									ImGui::SetItemDefaultFocus();
								}

								switch (proxy.type)
								{
								case 0:
								{
									SpriteAnimationAsset* animationAsset = proxy.spriteAnimation;

									for (const std::pair<std::string, SpriteAnimationSequence>& sequence : animationAsset->_Sequences())
									{
										bool selected = currentAnimation == sequence.first;

										if (ImGui::Selectable(sequence.first.c_str(), &selected))
										{
											animation->PlayAnimationLooped(sequence.first);
											ImGui::SetItemDefaultFocus();
										}
									}
									break;
								}
								case 1:
								{
									SpineAnimationAsset* animationAsset = proxy.spineAnimation;
									spine::Vector<spine::Animation*> sequences = animationAsset->GetAnimationStateData()->getSkeletonData()->getAnimations();

									for (unsigned i = 0; i < sequences.size(); i++)
									{
										bool selected = currentAnimation == sequences[i]->getName().buffer();

										if (ImGui::Selectable(sequences[i]->getName().buffer(), &selected))
										{
											animation->PlayAnimationLooped(sequences[i]->getName().buffer());
											ImGui::SetItemDefaultFocus();
										}
									}
									break;
								}
								default:
									break;
								}

								ImGui::EndCombo();
							}
						}

						ImGui::TreePop();
					}
				}
				else
				if (instance.get_value<Components::Component*>()->get_type() == rttr::type::get<Components::AnimationLooper>())
				{
					if (ImGui::TreeNodeEx("Animation Looper"))
					{
						using namespace Components;
						using namespace Graphics;
						AnimationLooper* animationLooper = instance.get_value<AnimationLooper*>();
						Animation* animation = animationLooper->Parent()->GetComponent<Animation>();
						AnimationProxy proxy = animation->GetAnimation();

						const std::string currentAnimation = animationLooper->GetSequence();

						if (ImGui::BeginCombo("Preview Sequence", currentAnimation.c_str(), ImGuiComboFlags_NoArrowButton))
						{
							bool selected = currentAnimation == "NONE";
							if (ImGui::Selectable("NONE", &selected))
							{
								animation->PlayAnimationLooped("NONE");
								ImGui::SetItemDefaultFocus();
							}

							switch (proxy.type)
							{
							case 0:
							{
								SpriteAnimationAsset* animationAsset = proxy.spriteAnimation;

								for (const std::pair<std::string, SpriteAnimationSequence>& sequence : animationAsset->_Sequences())
								{
									bool selected = currentAnimation == sequence.first;

									if (ImGui::Selectable(sequence.first.c_str(), &selected))
									{
										animationLooper->SetSequence(sequence.first);
										ImGui::SetItemDefaultFocus();
									}
								}
								break;
							}
							case 1:
							{
								SpineAnimationAsset* animationAsset = proxy.spineAnimation;
								spine::Vector<spine::Animation*> sequences = animationAsset->GetAnimationStateData()->getSkeletonData()->getAnimations();

								for (unsigned i = 0; i < sequences.size(); i++)
								{
									bool selected = currentAnimation == sequences[i]->getName().buffer();

									if (ImGui::Selectable(sequences[i]->getName().buffer(), &selected))
									{
										animationLooper->SetSequence(sequences[i]->getName().buffer());
										ImGui::SetItemDefaultFocus();
									}
								}
								break;
							}
							default:
								break;
							}

							ImGui::EndCombo();
						}
						ImGui::TreePop();
					}

				}
				else
				if (instance.get_value<Components::Component*>()->get_type() == rttr::type::get<Graphics::Camera>())
				{
					if (ImGui::TreeNodeEx(name.c_str()))
					{
						using namespace glm;
						using namespace Graphics;

						Camera* camera = instance.get_value<Camera*>();
						
						vec2 size = camera->GetSize();
						ImGui::InputFloat("Size", &size.x);
						camera->SetSize(size.x);

						ImGui::ColorPicker4("Background", &camera->background.r, ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR);

						ImGui::InputFloat("Track Rate", &camera->trackRate);

						ImGui::TreePop();
					}
				}
				else
				if (instance.get_value<Components::Component*>()->get_type() == rttr::type::get<ChaseScene::BossAnimationComponent>())
				{
					if (ImGui::TreeNodeEx(name.c_str()))
					{
						using namespace UI;
						using namespace ChaseScene;
						using namespace Graphics;
						using namespace ObjectManagement;

						BossAnimationComponent* animation = instance.get_value<BossAnimationComponent*>();
						FMODCore::Audio* instance = FMODCore::Audio::getAudioInstance();
						std::vector<std::string> audioList = instance->getEventListStrings();


						// SPINE ANIMATION -------------------------------------------------------------------------------------------
						std::vector<Object*>& animations = ObjectManagement::SpineAnimationManager::GetInstance()->GetList();
						SpineAnimationAsset* currentAnimation_ = animation->GetAnimationAsset();
						bool newAnimation_ = false;

						std::string name;
						if (currentAnimation_)
							name = currentAnimation_->GetName().c_str();
						else
							name = "NONE";

						//ImGui::ListBox("Texture", &currentTexture_, textureNames, textures.size());
						if (ImGui::BeginCombo("Spine Animation", name.c_str(), ImGuiComboFlags_NoArrowButton))
						{
							for (int n = 0; n < animations.size(); n++)
							{
								bool is_selected = (currentAnimation_ == animations[n]);
								if (ImGui::Selectable(animations[n]->GetName().c_str(), is_selected))
								{
									currentAnimation_ = (Graphics::SpineAnimationAsset*)animations[n];
									animation->SetAnimationAsset(currentAnimation_);
									newAnimation_ = true;
								}
								if (is_selected)
									ImGui::SetItemDefaultFocus();
							}
							ImGui::EndCombo();
						}

						if (newAnimation_)
						{
							animation->ResetAnimations();
						}

						// WORKING NODE -------------------------------------------------------------------------------------------
						if (BossAnimationNode* working = animation->GetWorkingNode())
						{
							if (ImGui::TreeNodeEx("Working Node"))
							{
								ImGui::InputFloat3("Position", &working->Position.x);

								ImGui::InputFloat("Time", &working->Time);

								std::string current = working->Animation;
								if (ImGui::BeginCombo("Animation", current.c_str()))
								{
									spine::Vector<spine::Animation*>& animations = currentAnimation_->GetAnimationStateData()->getSkeletonData()->getAnimations();

									bool selected = current == "NONE";

									if (ImGui::Selectable("NONE", &selected))
									{
										working->Animation = "NONE";
										ImGui::SetItemDefaultFocus();
									}

									for (int i = 0; i < animations.size(); i++)
									{
										selected = current == animations[i]->getName().buffer();
										if (ImGui::Selectable(animations[i]->getName().buffer(), &selected))
										{
											working->Animation = animations[i]->getName().buffer();
											ImGui::SetItemDefaultFocus();
										}
									}

									ImGui::EndCombo();
								}

								ImGui::Checkbox("Loop", &working->LoopAnimation);

								bool isCurve = working->GetIsCurve();
								if(ImGui::Checkbox("Is Curve", &isCurve))
									working->SetIsCurve(isCurve);

								if (isCurve)
								{
									ImGui::InputFloat3("Handle", &working->Handle.x);
								}

								int effectCount = working->Effects.size();
								if(ImGui::InputInt("Effect Count", &effectCount))
									working->Effects.resize(effectCount);

								for (int i = 0; i < working->Effects.size(); i++)
								{
									SoundEffect& effect = working->Effects[i];

									if (ImGui::TreeNodeEx(("Effect " + std::to_string(i)).c_str()))
									{
										if (ImGui::BeginCombo("Effect", effect.SoundEffectName.c_str()))
										{
											bool selected = effect.SoundEffectName == "NONE";

												if (ImGui::Selectable("NONE", &selected))
												{
													effect.SoundEffectName = "NONE";
														ImGui::SetItemDefaultFocus();
												}

											for (int j = 0; j < audioList.size(); j++)
											{
												selected = effect.SoundEffectName == audioList[j];

												if (ImGui::Selectable(audioList[j].c_str(), &selected))
												{
													effect.SoundEffectName = audioList[j];
													ImGui::SetItemDefaultFocus();
												}
											}

											ImGui::EndCombo();
										}

										ImGui::InputFloat("Offset", &effect.Offset);

										ImGui::TreePop();
									}
								}

								ImGui::TreePop();

							}
						}

						// EDITING VARS -------------------------------------------------------------------------------------------
						bool temp_bool = animation->GetAddNodes();
						if(ImGui::Checkbox("Add Node", &temp_bool))
							animation->SetAddNodes(temp_bool);

						temp_bool = animation->GetMoveWorkingNode();
						if(ImGui::Checkbox("Move Node", &temp_bool))
							animation->SetMoveWorkingNode(temp_bool);

						temp_bool = animation->GetMoveHandle();
						if (ImGui::Checkbox("Move Handle", &temp_bool))
							animation->SetMoveHandle(temp_bool);

						ImGui::TreePop();
					}

				}
				else
				if (ImGui::TreeNodeEx(name.c_str()))
				{
					rttr::array_range<rttr::property> propArray = instance.get_value<Components::Component*>()->get_type().get_properties();
					for (const rttr::property& currentProp : propArray)
					{
						std::string propName = currentProp.get_name().to_string();
						// ImGui::Text(propName.c_str());
						rttr::variant propVal = currentProp.get_value(instance);
						propVal.is_valid();
						CloudEngine::Editor::getBaseType(propVal, propName);
						instance.is_valid();
						currentProp.set_value(instance, propVal);
						instance.is_valid();
					}

					ImGui::TreePop();
				}

			}
			else
			{
				if (instance.is_type<CloudEngine::Graphics::TexturePtr>())
				{

				}
				if (instance.is_type<CloudEngine::Graphics::Sprite*>())
				{
					CloudEngine::Graphics::Sprite*& currentSprite = instance.get_value<CloudEngine::Graphics::Sprite*>();

					std::string name = "NONE";

					if (currentSprite)
					{
						name = currentSprite->GetName();
					}


					if (ImGui::BeginCombo("Sprite", name.c_str()))
					{
						bool selected = currentSprite == NULL;
						if (ImGui::Selectable("NONE", &selected))
						{
							currentSprite = NULL;
						}

						for (CloudEngine::ObjectManagement::Object* obj : CloudEngine::ObjectManagement::SpriteManager::GetInstance()->GetList())
						{
							CloudEngine::Graphics::Sprite* sprite = (CloudEngine::Graphics::Sprite*)obj;
							selected = sprite == currentSprite;

							if (ImGui::Selectable(sprite->GetName().c_str(), &selected))
							{
								currentSprite = sprite;
							}
						}
						ImGui::EndCombo();
					}
					return;
				}
				else
				{
					instance.is_valid();

					rttr::array_range<rttr::property> propArray = instance.get_type().get_properties();
					for (const rttr::property& currentProp : propArray)
					{
						std::string propName = currentProp.get_name().to_string();
						instance.is_valid();
						rttr::variant propVal = currentProp.get_value(instance);
						instance.is_valid();
						CloudEngine::Editor::getBaseType(propVal, propName);
						instance.is_valid();
						currentProp.set_value(instance, propVal); //is_valid changes!
						instance.is_valid();
					}
				}

				
			}
		}
	}

	/**
	 * Helper function for displaying text in ImGUI window.
	 * \param data - user data
	 * \return int
	 *
	 */
	static int InputTextCallback(ImGuiInputTextCallbackData* data)
	{
		InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			// Resize string callback
			// If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
			std::string* str = user_data->Str;
			IM_ASSERT(data->Buf == str->c_str());
			str->resize(data->BufTextLen);
			data->Buf = (char*)str->c_str();
		}
		else if (user_data->ChainCallback)
		{
			// Forward to user callback, if any
			data->UserData = user_data->ChainCallbackUserData;
			return user_data->ChainCallback(data);
		}
		return 0;
	}

	/**
	 * This function allows you to input text into an ImGui window.
	 * \param label - buffer of text
	 * \param str - string of text
	 * \param flags - ImGui flags to be set
	 * \param callback - ngl i have no idea what this does
	 * \param user_data - void ptr
	 * \return bool
	 *
	 */
	bool Editor::InputText(const char* label, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextCallback_UserData cb_user_data;
		cb_user_data.Str = str;
		cb_user_data.ChainCallback = callback;
		cb_user_data.ChainCallbackUserData = user_data;
		return ImGui::InputText(label, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
	}

	/**
	 * draws drop down to choose between different editor modes.
	 */
	void Editor::MenuBar::DrawModeHeader()
	{
		if (ImGui::BeginMenu("Modes"))
		{
			if (ImGui::MenuItem("Main Editor"))
			{
				EditorSystem::GetInstance()->CloseEditor<AudioEditor>();
				EditorSystem::GetInstance()->CloseEditor<ArtEditor>();
				mbInstance_->mainMode_ = true;
				mbInstance_->audioMode_ = false;
				mbInstance_->artMode_ = false;
			}
			if (ImGui::MenuItem("Audio Editor"))
			{
				EditorSystem::GetInstance()->OpenEditor<AudioEditor>();
				EditorSystem::GetInstance()->CloseEditor<ArtEditor>();
				mbInstance_->audioMode_ = true;
				mbInstance_->mainMode_ = false;
				mbInstance_->artMode_ = false;

			}
			if (ImGui::MenuItem("Art Editor"))
			{
				EditorSystem::GetInstance()->CloseEditor<AudioEditor>();
				EditorSystem::GetInstance()->OpenEditor<ArtEditor>();
				mbInstance_->audioMode_ = false;
				mbInstance_->mainMode_ = false;
				mbInstance_->artMode_ = true;
			}

			ImGui::EndMenu();
		}
	}
	/**
	 * draws menu bar
	 */
	void CloudEngine::Editor::MenuBar::Draw()
	{
		if (ImGui::BeginMainMenuBar())
		{
			getSelectedObject();
			DrawModeHeader();
			TranslateMode();
			ResizeMode();
			RotateMode();


		if (ImGui::Button(labelresize_.c_str()))//ImGui::MenuItem("Resize"))
		{
			resizeBtn_ = !resizeBtn_;
			if (resizeBtn_)
			{
				std::cout << "RESIZE MODE ON" << std::endl;
				labelresize_ = "Resize (ON)";
				labelrotate_ = "Rotate";
				labeltranslate_ = "Translate";

				translateBtn_ = false;
				rotateBtn_ = false;
			}
			else
			{
				std::cout << "RESIZE MODE OFF" << std::endl;
				labelresize_ = "Resize";
				//resizeTex = nullptr;
			}
		}
		if (ImGui::Button(labelrotate_.c_str()))//ImGui::MenuItem("Rotate"))
		{
			mbInstance_->rotateBtn_ = !mbInstance_->rotateBtn_;
			if (mbInstance_->rotateBtn_)
			{
				std::cout << "ROTATE ON" << std::endl;
				labelrotate_ = "Rotate (ON)";
				labelresize_ = "Resize";
				labeltranslate_ = "Translate";

				translateBtn_ = false;
				resizeBtn_ = false;
			}
			else
			{
				std::cout << "ROTATE OFF" << std::endl;
				labelrotate_ = "Rotate";
				//rotationTex = nullptr;
			}
		}
		if (ImGui::Button(labeltranslate_.c_str()))//ImGui::MenuItem("Translate"))
		{
			mbInstance_->translateBtn_ = !mbInstance_->translateBtn_;
			if (mbInstance_->translateBtn_)
			{
				std::cout << "TRANSLATE MODE ON" << std::endl;
				labeltranslate_ = "Translate (ON)";
				labelresize_ = "Resize";
				labelrotate_ = "Rotate";

				resizeBtn_ = false;
				rotateBtn_ = false;
			}
			else
			{
				std::cout << "TRANSLATE MODE OFF" << std::endl;
				labeltranslate_ = "Translate";
				//translationTex = nullptr;
			}
		}

			if (ImGui::BeginMenu("Actions"))
			{
				if (ImGui::MenuItem("Add Scene"))
				{
					ObjectManagement::FileManager::CreateFile<Scene::Scene>("./MetaData/Scenes/NewScene.scene");
				}
				if (ImGui::MenuItem("Save"))
				{
					Engine::sceneManager->GetCurrentScene()->Save();
				}
				if (ImGui::MenuItem("Show keyboard shortcuts"))
				{
					Editor::EditorSystem::GetInstance()->OpenEditor<Editor::QuickKeys>();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Graphics"))
			{
				if (ImGui::MenuItem("Post Processing..."))
				{
					EditorSystem::GetInstance()->OpenEditor<Graphics::PostProcessingEditor>();
				}

				ImGui::EndMenu();
			}

			Components::GameObject* selected = GetSelectedObject();
			if (selected)
			{
				using namespace Components;
				Transform* trans = selected->GetComponent<Transform>();

				if(trans)
					Debug::DrawRect(*trans->GetTranslation(), *trans->GetScale(), 0.0f, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
			}

			ImGui::EndMainMenuBar();
		}
	}

	/**
	 *  math for when user toggles resize
	 */
	void CloudEngine::Editor::MenuBar::ResizeMode()
	{
		if (!resizeBtn_) return;
		if (ImGui::GetIO().WantCaptureMouse) return;
		if (!InputController::leftMouseButtonPressed()) return;

		using namespace Components;
		using namespace Graphics;
		using namespace glm;

		Camera* active = Graphics::Camera::GetActiveCamera();
		if (active == NULL) return;

		Components::GameObject* selected = GetSelectedObject();
		if (selected == NULL) return;

		Transform* trans = selected->GetComponent<Transform>();
		mat4 worldToObj = inverse(*trans->GetMatrix());

		vec2 worldMousePos = active->GetScreenToWorldMatrix() * glm::vec4(InputController::getMousePos(), 0, 1);
		vec2 objMousePos = worldToObj * glm::vec4(worldMousePos, 0, 1);

		trans->SetScale(*trans->GetScale() * glm::vec3(objMousePos, 1));
	}

	/**
	* math for when user toggles translate mode
	*/
	void CloudEngine::Editor::MenuBar::TranslateMode()
	{
		if (!translateBtn_) return;
		if (ImGui::GetIO().WantCaptureMouse) return;
		if (!InputController::leftMouseButtonPressed()) return;

		using namespace Components;
		using namespace Graphics;
		using namespace glm;

		Camera* active = Graphics::Camera::GetActiveCamera();
		if (active == NULL) return;

		Components::GameObject* selected = GetSelectedObject();
		if (selected == NULL) return;

		Transform* trans = selected->GetComponent<Transform>();

		vec2 worldMousePos = active->GetScreenToWorldMatrix() * glm::vec4(InputController::getMousePos(), 0, 1);

		GameObject* parentObject;
		if ((parentObject = trans->Parent()->GetParent()) != NULL)
		{
			Transform* parentTrans = selected->GetComponent<Transform>();
			vec2 objectMousePos = glm::inverse(*parentTrans->GetMatrix()) * glm::vec4(worldMousePos, 0, 1);

			trans->SetTranslation(glm::vec3(objectMousePos, trans->GetTranslation()->z));
		}
		else
		{
			trans->SetTranslation(glm::vec3(worldMousePos, trans->GetTranslation()->z));
		}
	}
}

/**
* draws keyboard shortcut descriptions
*/
void CloudEngine::Editor::QuickKeys::Draw()
{
	ImGui::Text("Keyboard Shortcuts:");
	ImGui::Text("F1: Main Menu");
	ImGui::Text("F2: Toggle editor");
	ImGui::Text("F3: Hardcoded Designer Level");
	ImGui::Text("F4: Next checkpoint");
	ImGui::Text("F5: Previous checkpoint");
	ImGui::Text("F7: Chase Scene (goes to end if in chase scene)");
	ImGui::Text("F9: Show colliders");
	ImGui::Text("F11: Toggle fullscreen");
	ImGui::Text("N: Next Checkpoint");
	ImGui::Text("B: Last Checkpoint");
	ImGui::Text("LCtrl + S: Save Level");
	ImGui::Text("LCtrl + C: Copy selected obj");
	ImGui::Text("LCtrl + V: Paste selected obj");
}

/**
* math for when user toggles rotate mode
*/
void CloudEngine::Editor::MenuBar::RotateMode()
{
	if (!rotateBtn_) return;
	if (ImGui::GetIO().WantCaptureMouse) return;
	if (!InputController::leftMouseButtonPressed()) return;

	using namespace Components;
	using namespace Graphics;
	using namespace glm;

	Camera* active = Graphics::Camera::GetActiveCamera();
	if (active == NULL) return;

	Components::GameObject* selected = GetSelectedObject();
	if (selected == NULL) return;

	Transform* trans = selected->GetComponent<Transform>();
	mat4 worldToObj = inverse(*trans->GetMatrix());

	vec2 worldMousePos = active->GetScreenToWorldMatrix() * glm::vec4(InputController::getMousePos(), 0, 1);
	vec2 objectMousePos = worldToObj * glm::vec4(worldMousePos, 0, 1);

	float rotation = atan(objectMousePos.y, objectMousePos.x);

	trans->SetRotation(trans->GetRotation() - (degrees(rotation) - 90));
}

/**
* draws list of game objects in scene hierarchy window
*/
void CloudEngine::Editor::SceneHierarchyWindow::DrawGameObj(CloudEngine::Components::GameObject* obj)
{
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
	Components::GameObject* globalObj = GetSelectedObject();
	bool selected = globalObj == obj;
	if (selected)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
	}
	std::vector< Components::GameObject*>& children = obj->GetChildren();
	if (children.size() == 0)
	{
		flags |= ImGuiTreeNodeFlags_Bullet;
	}

	char label[128];
	std::string stringName = obj->GetName();
	stringName += "##";
	stringName += std::to_string(obj->GetID());
	sprintf_s(label, stringName.c_str());
	bool destroyObject = false;

	bool open = ImGui::TreeNodeEx(label, flags);
	bool clicked = ImGui::IsItemClicked();
	if (clicked)
	{
		if (selected)
		{
			globalObj = NULL;
		}
		else
		{
			globalObj = obj;
		}

		Editor::InspectorWindow* inspector = Editor::EditorSystem::GetInstance()->OpenEditor<Editor::InspectorWindow>();

		SetSelectedObject(globalObj);
	}

	if (open)
	{
		for (Components::GameObject* child : children)
		{
			DrawGameObj(child);
		}
		ImGui::TreePop();
	}
}

/**
* draws scene hierarchy window
*/
void CloudEngine::Editor::SceneHierarchyWindow::Draw()
{
	Components::GameObject* globalObj = GetSelectedObject();
	// EditorWindow("Scene Hierarchy", CloudEngine::Engine::Core::WindowWidth(), CloudEngine::Engine::Core::WindowHeight() * 40, 
	//CloudEngine::Engine::Core::WindowWidth(), CloudEngine::Engine::Core::WindowHeight(), false, true, false) 
	isCopied();
	isPasted();
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Add Object"))
			{
				Components::GameObject* newObj = new Components::GameObject();
				newObj->AddComponent<Components::Transform>();
				Components::Transform* newObjTransform = newObj->GetComponent<Components::Transform>();
				glm::vec2 worldPos = Graphics::Camera::GetActiveCamera()->GetScreenToWorldMatrix() * glm::vec4(Engine::Core::WindowWidth() / 2, Engine::Core::WindowHeight() / 2, 0.0f, 1.0f);
				newObjTransform->SetTranslation(glm::vec3(worldPos, 0.0f));
				Components::AddObjectInManager(newObj, Components::ManagerType::M_ACTIVE_LIST);
			}
			if (ImGui::MenuItem("Add Prefab"))
			{
				Editor::AddPrefabWindow* pwin = Editor::EditorSystem::GetInstance()->OpenEditor<Editor::AddPrefabWindow>();
			}
			if (ImGui::MenuItem("Remove Object"))
			{

				if (globalObj)
				{
					globalObj->Destroy();
				}
				SetSelectedObject(NULL);

				Editor::InspectorWindow* inspector = Editor::EditorSystem::GetInstance()->OpenEditor<Editor::InspectorWindow>();

				globalObj = NULL;
			}
			if (globalObj)
			{
				if (ImGui::MenuItem("Add Child Object"))
				{
					Components::GameObject* newObj = new Components::GameObject();
					newObj->AddComponent<Components::Transform>();
					Components::AddObjectInManager(newObj, Components::ManagerType::M_ACTIVE_LIST);
					globalObj->AddChild(newObj);
				}
				if (ImGui::MenuItem("Save Object as Prefab"))
				{
					globalObj->Serialize();
				}
			}
			ImGui::EndMenu();
		}


		ImGui::EndMenuBar();
	}

	// draw gameobject tree
	int numActiveList = CloudEngine::Components::GetObjectCountOfManager(CloudEngine::Components::M_ACTIVE_LIST);
	std::vector<Components::GameObject*>& objList = CloudEngine::Components::GetActiveList();

	for (int i = 0; i < numActiveList; i++)
	{
		if (objList[i]->GetParent() == NULL)
		{
			DrawGameObj(objList[i]);
		}
	}
}

/**
* draws file system
*/
void CloudEngine::Editor::FileSystemWindow::Draw()
{

	static constexpr ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH
		| ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;

	static const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
	if (ImGui::BeginTable("Project", 3, flags))
	{
		ImGui::TableSetupScrollFreeze(0, 1);
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
		ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 12.0f);
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
		ImGui::TableHeadersRow();

		DrawDir(dir);

		ImGui::EndTable();
	}

}

/**
* helper for file system
*/
void CloudEngine::Editor::FileSystemWindow::HandleMessage(Messaging::Message* msg)
{
	// check if this was a file load message
	if (msg->GetMessageType() != Messaging::MESSAGE_TYPE::TYPE_FILE_LOAD) return;

	// retrieves file data from the message
	Messaging::FileMessage* fileMessage = (Messaging::FileMessage*)msg;
	ObjectManagement::FileObject* file = fileMessage->GetFile();
	const std::filesystem::path& filePath = file->FilePath();

	// build file tree from filepaths

	AddFile(file);


}

/**
* enum for formats for file sizes
*/
static std::string formats[] =
{
	"%d B",
	"%d KB",
	"%d MB",
	"%d GB",
	"%d TB",
};

/**
* converts bytes to bigger chunks
*/
std::pair<std::string, long long> CloudEngine::Editor::FileSystemWindow::CleanSize(long long bytes)
{
	std::pair<std::string, long long> pair = { formats[0], bytes };
	int i = 0;

	while (bytes / 1024 > 0)
	{
		bytes /= 1024;
		i++;
	}
	pair.first = formats[i];
	pair.second = bytes;

	return pair;
}

/**
* function to rename files in filesystem
*/
std::string CloudEngine::Editor::FileOptionsWindow::Rename(ObjectManagement::FileObject* obj, std::string newName)
{
	std::string newStr;
	std::string dir = obj->Directory().string();
	std::string ext = (obj->FilePath().extension()).string();
	newStr = (dir + newName) + ext;
	obj->Rename(newName);

	return newStr;
}

/**
* draws file options menu
*/
void CloudEngine::Editor::FileOptionsWindow::Draw()
{
	Components::GameObject* globalObj = GetSelectedObject();
	if (ImGui::Selectable("Delete"))
	{

		Editor::EditorSystem::GetInstance()->OpenEditor<Editor::FileSystemWindow>()->RemoveFile(activeFile);
		ObjectManagement::FileObject::RemoveFile(activeFile);
		Editor::EditorSystem::GetInstance()->CloseEditor<Editor::FileOptionsWindow>();
	}
	char namebuf[256];
	strcpy(namebuf, activeFile->GetName().c_str());
	if (ImGui::InputText("Rename", namebuf, 256))
	{
		Rename(activeFile, (std::string)namebuf);
	}

	if (activeFile->FilePath().extension() == ".prefab")
	{
		if (ImGui::Selectable("Add Prefab to Scene"))
		{
			Components::GameObject* clone = activeFile->GetObject<Components::GameObject>()->Clone();
			Components::Transform* cloneTransform = clone->GetComponent<Components::Transform>();
			glm::vec2 worldPos = Graphics::Camera::GetActiveCamera()->GetScreenToWorldMatrix() * glm::vec4(Engine::Core::WindowWidth() / 2, Engine::Core::WindowHeight() / 2, 0.0f, 1.0f);
			cloneTransform->SetTranslation(glm::vec3(worldPos, 0.0f));
			Components::AddObjectInManager(clone, Components::M_ACTIVE_LIST);
		}
		if (globalObj)
		{
			if (ImGui::MenuItem("Add as Child to Selected Object"))
			{
				Components::GameObject* clone = activeFile->GetObject<Components::GameObject>()->Clone();
				Components::AddObjectInManager(clone, Components::M_ACTIVE_LIST);
				globalObj->AddChild(clone);
			}
		}
	}


	if (activeFile->FilePath().extension() == ".ani")
	{
		if (ImGui::Selectable("Edit Animation"))
		{
			SpriteAnimationEditor* aniEditor = Editor::EditorSystem::GetInstance()->OpenEditor<Editor::SpriteAnimationEditor>();
			aniEditor->SetAnimation(activeFile->GetObject<Graphics::SpriteAnimationAsset>());
		}
	}

	if (activeFile->FilePath().extension() == ".spine")
	{
		if (ImGui::Selectable("View Spine Animation"))
		{
			//probably write separate function that gets spine animation associated with selected file so it opens on that animation.

			spriteViewer = Editor::EditorSystem::GetInstance()->OpenEditor<Editor::SpineAnimationViewer>();
			spriteViewer->SetAnimation(activeFile->GetObject<Graphics::SpineAnimationAsset>());
		}
	}

	if (activeFile->FilePath().extension() == ".sprite") //sprite viewer
	{
		if (ImGui::Selectable("Edit Sprite"))
		{
			//probably write separate function that gets sprite associated with selected file so it opens on that sprite.
			if (activeFile->GetObject<Graphics::Sprite>())
			{

				SpriteEditor* spriteEditor = Editor::EditorSystem::GetInstance()->OpenEditor<Editor::SpriteEditor>();
				spriteEditor->SetSprite(activeFile->GetObject<Graphics::Sprite>());
			}
		}
	}
	if (activeFile->FilePath().extension() == ".png")
	{
		if (ImGui::Selectable("View Image"))
		{
			TextureEditorWindow* textureEditor = Editor::EditorSystem::GetInstance()->OpenEditor<Editor::TextureEditorWindow>();
			textureEditor->SetTexture(activeFile->GetObject<Graphics::Texture>());
			//open texture viewer
		}
	}

	if (activeFile->FilePath().extension() == ".scene")
	{
		if (ImGui::Selectable("Switch Scenes"))
		{
			Engine::sceneManager->SwitchTo(activeFile->GetName());
			Editor::EditorSystem::GetInstance()->CloseEditor<Editor::FileOptionsWindow>();
		}
	}

}

/**
* draws file within file system
*/
void CloudEngine::Editor::FileSystemWindow::DrawFile(ObjectManagement::FileObject* fileObj)
{
	std::string name = fileObj->GetName();
	std::pair<std::string, long long> pair = CleanSize(fileObj->Size());

	//ImGui::TableNextColumn();
	ImGui::TableNextColumn();
	ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);

	if (ImGui::IsItemClicked())
	{
		//do stuff
		activeFile = fileObj;
		fso = Editor::EditorSystem::GetInstance()->OpenEditor<Editor::FileOptionsWindow>();

	}

	ImGui::TableNextColumn();
	ImGui::Text((pair.first).c_str(), pair.second);
	ImGui::TableNextColumn();
	ImGui::TextUnformatted(fileObj->Extension().string().c_str());

}

/**
* draws folders within folder system
*/
void CloudEngine::Editor::FileSystemWindow::DrawDir(const CloudEngine::Directory& d)
{
	std::string name = d.GetName();

	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	bool op = ImGui::TreeNodeEx(name.c_str());
	ImGui::TableNextColumn();
	ImGui::TextDisabled(" ");
	ImGui::TableNextColumn();
	ImGui::TextUnformatted("FOLDER");
	if (op)
	{
		for (const CloudEngine::Directory& subs : d.GetSubdirectories())
		{
			DrawDir(subs);
		}
		for (CloudEngine::ObjectManagement::FileObject* file : d.GetFiles())
		{
			DrawFile(file);
		}
		ImGui::TreePop();
	}



}

/**
* gets directory list
*/
std::vector<std::filesystem::path> GetDirectoryList(std::filesystem::path path)
{
	std::vector<std::filesystem::path> directory;
	if (path.has_parent_path())
	{
		directory = GetDirectoryList(path.parent_path());
	}
	directory.push_back(path);
	return directory;
}

/**
* adds given file to directory
*/
void CloudEngine::Editor::FileSystemWindow::AddFile(ObjectManagement::FileObject* fileObj)
{
	std::vector<std::filesystem::path> path = GetDirectoryList(fileObj->FilePath());

	Directory* current = &dir;

	for (unsigned i = 1; i < path.size() - 1; i++)
	{
		current = &current->AddDirectory(path[i]);
	}

	current->GetFiles().push_back(fileObj);
}

/**
* removes given file from directory
*/
void CloudEngine::Editor::FileSystemWindow::RemoveFile(ObjectManagement::FileObject* fileObj)
{
	std::vector<std::filesystem::path> path = GetDirectoryList(fileObj->FilePath());

	Directory* current = &dir;

	if (path.size() == 0) return;

	for (int i = 1; i < path.size() - 1; i++)
	{
		current = &current->AddDirectory(path[i]);
	}

	current->RemoveFile(fileObj);
}

/**
* draws inspector window
*/
void CloudEngine::Editor::InspectorWindow::Draw()
{
	Components::GameObject* activeObj_ = GetSelectedObject();
	if (activeObj_ == NULL)
	{
		return;
	}
	char namebuf[256];
	strcpy(namebuf, activeObj_->GetName());
	ImGui::InputText("Name", namebuf, 256);
	activeObj_->SetName(namebuf);

	int layernum = activeObj_->GetLayerNumber();
	ImGui::InputInt("Layer", &layernum);
	activeObj_->SetLayerNumber(layernum);

	Editor::AddComponentWindow* addComp;
	if (ImGui::Button("Add/Remove Components", ImVec2(200.0, 25.0)))
	{
		addComp = Editor::EditorSystem::GetInstance()->OpenEditor<Editor::AddComponentWindow>();
	}



	std::vector<Components::Component*>& components = activeObj_->GetComponents();
	for (Components::Component* comp : components)
	{
		std::string name = comp->get_type().get_name().to_string();
		rttr::variant component = comp;

		getBaseType(component, name);
	}

	SetSelectedObject(activeObj_);
}

/**
* draws window for adding components
*/
void CloudEngine::Editor::AddComponentWindow::Draw()
{
	if (ImGui::TreeNodeEx("Choose a Component: "))
	{
		// bool isOpen = false;
		Components::GameObject* active = GetSelectedObject();
		std::unordered_map<rttr::type, bool> addedComponents;

		rttr::array_range<rttr::type> componentsArr = rttr::type::get<CloudEngine::Components::Component>().get_derived_classes();
		for (rttr::type component : componentsArr)
		{
			if (component == rttr::type::get<Components::Behavior>()) continue;
			if (component == rttr::type::get<Physics::Collider>()) continue;

			addedComponents[component] = ComponentLookup(component, active);
		}
		for (rttr::type currComp : componentsArr)
		{
			if (currComp == rttr::type::get<Components::Behavior>()) continue;
			if (currComp == rttr::type::get<Physics::Collider>()) continue;

			if (ImGui::Checkbox(currComp.get_name().data(), &addedComponents[currComp]))
			{
				if (addedComponents[currComp])
				{
					addComponent(currComp, active);
				}
				else
				{
					removeComponent(currComp, active);
				}
			}
		}

		ImGui::TreePop();

		if (ImGui::Button("Done"))
		{
			Editor::EditorSystem::GetInstance()->CloseEditor<Editor::AddComponentWindow>();
		}
		if (ImGui::Button("Draw custom collider"))
		{
			
		}
	}
}

/**
* draws window that opens when adding prefabs
*/
void CloudEngine::Editor::AddPrefabWindow::Draw()
{
	bool op = false;
	std::vector<ObjectManagement::FileObject*> prefabs = ObjectManagement::FileManager::GetSingletonInstance()->GetSpecificFilesSuffix(".prefab");
	for (unsigned i = 0; i < prefabs.size(); i++)
	{
		std::string name = RemoveExtension(prefabs[i]->GetName());
		if (ImGui::Selectable(name.c_str(), op))
		{
			Components::GameObject* obj = prefabs[i]->GetObject<Components::GameObject>()->Clone();
			Components::Transform* cloneTransform = obj->GetComponent<Components::Transform>();
			glm::vec2 worldPos = Graphics::Camera::GetActiveCamera()->GetScreenToWorldMatrix() * glm::vec4(Engine::Core::WindowWidth() / 2, Engine::Core::WindowHeight() / 2, 0.0f, 1.0f);
			cloneTransform->SetTranslation(glm::vec3(worldPos, 0.0f));
			Components::AddObjectInManager(obj, Components::M_ACTIVE_LIST);
			Editor::EditorSystem::GetInstance()->CloseEditor<Editor::AddPrefabWindow>();
			break;
		}
	}
}

/**
* removes file extensions from prefab filenames
*/
std::string CloudEngine::Editor::AddPrefabWindow::RemoveExtension(std::string str)
{
	int period = str.find_last_of('.');
	std::string raw = str.substr(0, period);
	return raw;
}
