//
// Copyright (c) 2008-2022 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Input/FreeFlyController.h>

#include "Mover.h"
#include "SkeletalAnimation.h"

#include <Urho3D/DebugNew.h>


SkeletalAnimation::SkeletalAnimation(Context* context)
    : Sample(context)
    , drawDebug_(false)
{
    // Register an object factory for our custom Mover3D component so that we can create them to scene nodes
    if (!context->IsReflected<Mover3D>())
        context->AddFactoryReflection<Mover3D>();
}

void SkeletalAnimation::Start()
{
    // Execute base class startup
    Sample::Start();

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateInstructions();

    // Setup the viewport for displaying the scene
    SetupViewport();

    // Hook up to the frame update and render post-update events
    SubscribeToEvents();

    // Set the mouse mode to use in the sample
    SetMouseMode(MM_RELATIVE);
    SetMouseVisible(false);
}

void SkeletalAnimation::CreateScene()
{
    auto* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);

    // Create octree, use default volume (-1000, -1000, -1000) to (1000, 1000, 1000)
    // Also create a DebugRenderer component so that we can draw debug geometry
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<DebugRenderer>();

    // Create scene node & StaticModel component for showing a static plane
    Node* planeNode = scene_->CreateChild("Plane");
    planeNode->SetScale(Vector3(50.0f, 1.0f, 50.0f));
    auto* planeObject = planeNode->CreateComponent<StaticModel>();
    planeObject->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
    planeObject->SetMaterial(cache->GetResource<Material>("Materials/StoneTiled.xml"));

    // Create a Zone component for ambient lighting & fog control
    Node* zoneNode = scene_->CreateChild("Zone");
    auto* zone = zoneNode->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Color(0.5f, 0.5f, 0.5f));
    zone->SetFogColor(Color(0.4f, 0.5f, 0.8f));
    zone->SetFogStart(100.0f);
    zone->SetFogEnd(300.0f);

    // Create a directional light to the world. Enable cascaded shadows on it
    Node* lightNode = scene_->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f));
    auto* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetCastShadows(true);
    light->SetColor(Color(0.5f, 0.5f, 0.5f));
    light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
    // Set cascade splits at 10, 50 and 200 world units, fade shadows out at 80% of maximum shadow distance
    light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));

    // Create animated models
    const unsigned NUM_MODELS = 30;
    const float MODEL_MOVE_SPEED = 2.0f;
    const float MODEL_ROTATE_SPEED = 100.0f;
    const BoundingBox bounds(Vector3(-20.0f, 0.0f, -20.0f), Vector3(20.0f, 0.0f, 20.0f));

    for (unsigned i = 0; i < 1/*NUM_MODELS*/; ++i)
    {
        Node* modelNode = scene_->CreateChild("Jill");
        //modelNode->SetPosition(Vector3(Random(40.0f) - 20.0f, 0.0f, Random(40.0f) - 20.0f));
        modelNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
        //modelNode->SetRotation(Quaternion(0.0f, Random(360.0f), 0.0f));

        auto* modelObject = modelNode->CreateComponent<AnimatedModel>();

        //modelObject->SetModel(cache->GetResource<Model>("Models/Kachujin/Kachujin.mdl"));
        //modelObject->SetMaterial(cache->GetResource<Material>("Models/Kachujin/Materials/Kachujin.xml"));

        modelObject->SetModel(cache->GetResource<Model>("Models/Combat_walk_forward.fbx.d/Models/Soldier_body.mdl"));
        modelObject->SetMaterial(cache->GetResource<Material>("Models/Combat_walk_forward.fbx.d/Materials/Soldier_body1_LitNormalMap.xml"));

        Bone* bone = 0;
        Node* neck = 0;
        if (bone = modelObject->GetSkeleton().GetBone("Neck1"))
            neck = bone->node_;

        if (neck) {
            auto* objectHead = neck->CreateComponent<AnimatedModel>();
            objectHead->SetModel(cache->GetResource<Model>("Models/Combat_walk_forward.fbx.d/Models/Soldier_head.mdl"));
            objectHead->SetMaterial(cache->GetResource<Material>("Models/Combat_walk_forward.fbx.d/Materials/Soldier_head6_LitNormalMap.xml"));

            auto* headNode_ = objectHead->GetNode();
            headNode_->SetPosition(Vector3(0.0, -1.48, 0.0));
        }

        Bone* bone1 = 0;
        Node* gunNode_ = 0;
        if (bone1 = modelObject->GetSkeleton().GetBone("LeftHandIndex1")) {
            gunNode_ = bone1->node_->CreateChild("Gun");

            gunNode_->SetPosition(Vector3(-0.15f, -0.15f, 0.0f));
            auto* model = gunNode_->CreateComponent<StaticModel>();
            model->SetModel(cache->GetResource<Model>("Models/ar_style_gun.fbx.d/Models/ar15.mdl"));
            model->SetMaterial(cache->GetResource<Material>("Models/ar_style_gun.fbx.d/Materials/bullets_Lit.xml"));

            model->SetCastShadows(true);

            auto q = Quaternion(0.0, 0.0, -45.0);
            gunNode_->SetRotation(q);
            gunNode_->SetScale(12.0f);
        }

        modelObject->SetCastShadows(true);

        // Create an AnimationState for a walk animation. Its time position will need to be manually updated to advance the
        // animation, The alternative would be to use an AnimationController component which updates the animation automatically,
        // but we need to update the model's position manually in any case

        //auto* walkAnimation = cache->GetResource<Animation>("Models/Kachujin/Kachujin_Walk.ani");
        auto* walkAnimation = cache->GetResource<Animation>("Models/Combat_walk_forward.fbx.d/Animations/mixamo.com.ani");

        const float startTime = Random(walkAnimation->GetLength());

        auto animationController = modelNode->CreateComponent<AnimationController>();
        animationController->PlayNewExclusive(AnimationParameters{walkAnimation}.Looped().Time(startTime));

        // Create our custom Mover3D component that will move & animate the model during each frame's update
        auto* mover = modelNode->CreateComponent<Mover3D>();
        mover->SetParameters(MODEL_MOVE_SPEED, MODEL_ROTATE_SPEED, bounds);
    }

    // Create the camera. Limit far clip distance to match the fog
    cameraNode_ = scene_->CreateChild("Camera");
    cameraNode_->CreateComponent<FreeFlyController>();
    auto* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(300.0f);

    // Set an initial position for the camera scene node above the plane
    cameraNode_->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
}

void SkeletalAnimation::CreateInstructions()
{
    auto* cache = GetSubsystem<ResourceCache>();
    auto* ui = GetSubsystem<UI>();

    // Construct new Text object, set string to display and font to use
    auto* instructionText = GetUIRoot()->CreateChild<Text>();
    instructionText->SetText(
        "Use WASD keys and mouse/touch to move\n"
        "Space to toggle debug geometry"
    );
    instructionText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
    // The text has multiple rows. Center them in relation to each other
    instructionText->SetTextAlignment(HA_CENTER);

    // Position the text relative to the screen center
    instructionText->SetHorizontalAlignment(HA_CENTER);
    instructionText->SetVerticalAlignment(VA_CENTER);
    instructionText->SetPosition(0, GetUIRoot()->GetHeight() / 4);
}

void SkeletalAnimation::SetupViewport()
{
    auto* renderer = GetSubsystem<Renderer>();

    // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    SetViewport(0, viewport);
}

void SkeletalAnimation::SubscribeToEvents()
{
    // Subscribe HandlePostRenderUpdate() function for processing the post-render update event, sent after Renderer subsystem is
    // done with defining the draw calls for the viewports (but before actually executing them.) We will request debug geometry
    // rendering during that event
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(SkeletalAnimation, HandlePostRenderUpdate));
}

void SkeletalAnimation::Update(float timeStep)
{
    // Do not move if the UI has a focused element (the console)
    if (GetSubsystem<UI>()->GetFocusElement())
        return;

    auto* input = GetSubsystem<Input>();

    // Toggle debug geometry with space
    if (input->GetKeyPress(KEY_SPACE))
        drawDebug_ = !drawDebug_;
}

void SkeletalAnimation::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    // If draw debug mode is enabled, draw viewport debug geometry, which will show eg. drawable bounding boxes and skeleton
    // bones. Note that debug geometry has to be separately requested each frame. Disable depth test so that we can see the
    // bones properly
    if (drawDebug_)
        GetSubsystem<Renderer>()->DrawDebugGeometry(false);
}
