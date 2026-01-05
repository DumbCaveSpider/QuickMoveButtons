#include "./QuickMove.hpp"

#include <Geode/Geode.hpp>

#include <Geode/ui/GeodeUI.hpp>

#include <Geode/utils/NodeIDs.hpp>
#include <Geode/utils/terminate.hpp>

#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>

#include <Geode/binding/EditorUI.hpp>

using namespace geode::prelude;
using namespace quickmove;

// it's modding time :3
static auto qmbMod = getMod();

class $modify(MyEditorUI, EditorUI) {
    struct Fields {
        MoveSize moveSize = MoveSize::Small;

        CCMenu* m_buttonMenu;
        CCScale9Sprite* m_buttonMenuBg;
        CCScale9Sprite* m_buttonMenuBgDepth; // Second background for depth effect

        // Move buttons
        CCMenuItemSpriteExtra* m_moveUpBtn;
        CCMenuItemSpriteExtra* m_moveDownBtn;
        CCMenuItemSpriteExtra* m_moveLeftBtn;
        CCMenuItemSpriteExtra* m_moveRightBtn;

        CCMenuItemSpriteExtra* m_moveSizeBtn;

        // Rotation buttons (next to up arrow)
        CCMenuItemSpriteExtra* m_rotateClockwiseBtn;
        CCMenuItemSpriteExtra* m_rotateCounterClockwiseBtn;

        // Flip buttons (next to down arrow)
        CCMenuItemSpriteExtra* m_flipXBtn;
        CCMenuItemSpriteExtra* m_flipYBtn;

        // Icon sprites for updating when move size changes
        CCSprite* m_moveUpIcon;
        CCSprite* m_moveDownIcon;
        CCSprite* m_moveLeftIcon;
        CCSprite* m_moveRightIcon;

        // Button background sprites for visibility control
        ButtonSprite* m_moveUpBtnBg;
        ButtonSprite* m_moveDownBtnBg;
        ButtonSprite* m_moveLeftBtnBg;
        ButtonSprite* m_moveRightBtnBg;

        ButtonSprite* m_rotateClockwiseBtnBg;
        ButtonSprite* m_rotateCounterClockwiseBtnBg;

        ButtonSprite* m_flipXBtnBg;
        ButtonSprite* m_flipYBtnBg;

        // Dragging functionality
        bool m_isDragging = false;
        bool m_dontSwipe = false;

        CCPoint m_touchStartPos;
        CCPoint m_menuStartPos;

        // Track UI visibility state
        bool m_isUIVisible = true;
    };

    bool init(LevelEditorLayer * p0) {
        if (!EditorUI::init(p0)) return false;

        // create menu for the buttons
        m_fields->m_buttonMenu = CCMenu::create();
        m_fields->m_buttonMenu->setID("move-menu"_spr);

        // Get saved position or use center of screen as default
        float savedX = qmbMod->getSavedValue<float>("menuPositionX", CCDirector::sharedDirector()->getWinSize().width / 2);
        float savedY = qmbMod->getSavedValue<float>("menuPositionY", CCDirector::sharedDirector()->getWinSize().height / 2);

        m_fields->m_buttonMenu->setPosition(CCPoint(savedX, savedY));
        m_fields->m_buttonMenu->setAnchorPoint({ 0.5f, 0.5f });

        // Get background size from settings
        int bgSize = static_cast<int>(qmbMod->getSettingValue<int64_t>("scale-bg"));
        m_fields->m_buttonMenu->setContentSize({ static_cast<float>(bgSize), static_cast<float>(bgSize) });

        m_fields->m_buttonMenu->ignoreAnchorPointForPosition(false);
        m_fields->m_buttonMenu->setVisible(false); // Initially invisible since no objects are selected

        CCSize const depthSize = { static_cast<float>(bgSize + 6), static_cast<float>(bgSize + 6) }; // Larger for depth

        // create depth background for the menu (behind the main background)
        auto buttonMenuBgDepth = CCScale9Sprite::create("square02_001.png");
        buttonMenuBgDepth->setContentSize(depthSize);
        buttonMenuBgDepth->ignoreAnchorPointForPosition(false);
        buttonMenuBgDepth->setAnchorPoint({ 0.5, 0.5 });
        buttonMenuBgDepth->setOpacity(25); // Lower opacity for depth effect
        buttonMenuBgDepth->setPosition({ m_fields->m_buttonMenu->getContentWidth() / 2.f, m_fields->m_buttonMenu->getContentHeight() / 2.f }); // Same position as main background
        buttonMenuBgDepth->setColor({ 0, 0, 0 }); // Darker color for shadow effect

        m_fields->m_buttonMenuBgDepth = buttonMenuBgDepth;
        m_fields->m_buttonMenu->addChild(buttonMenuBgDepth);

        // create visible background for the menu (main background)
        auto buttonMenuBg = CCScale9Sprite::create("square02_001.png");
        buttonMenuBg->setContentSize(m_fields->m_buttonMenu->getContentSize());
        buttonMenuBg->ignoreAnchorPointForPosition(false);
        buttonMenuBg->setAnchorPoint({ 0.5, 0.5 });
        buttonMenuBg->setOpacity(50); // Set opacity to 50 when visible
        buttonMenuBg->setPosition({ m_fields->m_buttonMenu->getContentWidth() / 2.f, m_fields->m_buttonMenu->getContentHeight() / 2.f });

        m_fields->m_buttonMenuBg = buttonMenuBg;
        m_fields->m_buttonMenu->addChild(buttonMenuBg);

        // Get current move size icon info
        auto const iconInfo = getMoveSizeIconInfo(m_fields->moveSize);
        auto const moveBtnIconSpriteName = iconInfo.first;
        auto iconScale = iconInfo.second;

        // create the up btn
        auto upBtnSprite = ButtonSprite::create("", 20, true, "bigFont.fnt", "GJ_button_01.png", 30, 0.1f);
        m_fields->m_moveUpBtnBg = upBtnSprite; // Store reference

        m_fields->m_moveUpBtn = CCMenuItemSpriteExtra::create(
            upBtnSprite,
            this,
            menu_selector(MyEditorUI::onMoveUpButton)
        );
        m_fields->m_moveUpBtn->setID("move-up");
        m_fields->m_moveUpBtn->ignoreAnchorPointForPosition(false);
        m_fields->m_moveUpBtn->setAnchorPoint({ 0.5, 0.5 });
        m_fields->m_moveUpBtn->setPosition({ m_fields->m_buttonMenu->getContentWidth() / 2.f, (m_fields->m_buttonMenu->getContentHeight() / 2.f) + 35.f });

        // move up button
        auto moveUpBtnIcon = CCSprite::createWithSpriteFrameName(moveBtnIconSpriteName.c_str());
        moveUpBtnIcon->setScale(iconScale);
        moveUpBtnIcon->setAnchorPoint({ 0.5, 0.5 });
        moveUpBtnIcon->ignoreAnchorPointForPosition(false);
        moveUpBtnIcon->setPosition({ m_fields->m_moveUpBtn->getContentWidth() / 2.f, m_fields->m_moveUpBtn->getContentHeight() / 2.f });

        m_fields->m_moveUpIcon = moveUpBtnIcon; // Store reference

        m_fields->m_moveUpBtn->addChild(moveUpBtnIcon);
        m_fields->m_buttonMenu->addChild(m_fields->m_moveUpBtn);

        // create the down btn
        auto downBtnSprite = ButtonSprite::create("", 20, true, "bigFont.fnt", "GJ_button_01.png", 30, 0.1f);
        m_fields->m_moveDownBtnBg = downBtnSprite; // Store reference

        m_fields->m_moveDownBtn = CCMenuItemSpriteExtra::create(
            downBtnSprite,
            this,
            menu_selector(MyEditorUI::onMoveDownButton)
        );
        m_fields->m_moveDownBtn->setID("move-down");
        m_fields->m_moveDownBtn->ignoreAnchorPointForPosition(false);
        m_fields->m_moveDownBtn->setAnchorPoint({ 0.5, 0.5 });
        m_fields->m_moveDownBtn->setPosition({ m_fields->m_buttonMenu->getContentWidth() / 2.f, (m_fields->m_buttonMenu->getContentHeight() / 2.f) - 35.f });

        // move down button icon
        auto moveDownBtnIcon = CCSprite::createWithSpriteFrameName(moveBtnIconSpriteName.c_str());
        moveDownBtnIcon->setScale(iconScale);
        moveDownBtnIcon->setAnchorPoint({ 0.5, 0.5 });
        moveDownBtnIcon->ignoreAnchorPointForPosition(false);
        moveDownBtnIcon->setPosition({ m_fields->m_moveDownBtn->getContentWidth() / 2.f, m_fields->m_moveDownBtn->getContentHeight() / 2.f });
        moveDownBtnIcon->setFlipY(true);

        m_fields->m_moveDownIcon = moveDownBtnIcon; // Store reference

        m_fields->m_moveDownBtn->addChild(moveDownBtnIcon);
        m_fields->m_buttonMenu->addChild(m_fields->m_moveDownBtn);

        // create the left btn
        auto leftBtnSprite = ButtonSprite::create("", 15, true, "bigFont.fnt", "GJ_button_01.png", 35.f, 0.2f);
        m_fields->m_moveLeftBtnBg = leftBtnSprite; // Store reference
        m_fields->m_moveLeftBtn = CCMenuItemSpriteExtra::create(
            leftBtnSprite,
            this,
            menu_selector(MyEditorUI::onMoveLeftButton)
        );
        m_fields->m_moveLeftBtn->setID("move-left");
        m_fields->m_moveLeftBtn->ignoreAnchorPointForPosition(false);
        m_fields->m_moveLeftBtn->setAnchorPoint({ 0.5, 0.5 });
        m_fields->m_moveLeftBtn->setPosition({ (m_fields->m_buttonMenu->getContentWidth() / 2.f) - 35.f, m_fields->m_buttonMenu->getContentHeight() / 2.f });

        // move left button
        auto moveLeftBtnIcon = CCSprite::createWithSpriteFrameName(moveBtnIconSpriteName.c_str());
        moveLeftBtnIcon->setScale(iconScale);
        moveLeftBtnIcon->setAnchorPoint({ 0.5, 0.5 });
        moveLeftBtnIcon->ignoreAnchorPointForPosition(false);
        moveLeftBtnIcon->setPosition({ m_fields->m_moveLeftBtn->getContentWidth() / 2.f, m_fields->m_moveLeftBtn->getContentHeight() / 2.f });
        moveLeftBtnIcon->setRotation(-90.f);

        m_fields->m_moveLeftIcon = moveLeftBtnIcon; // Store reference

        m_fields->m_moveLeftBtn->addChild(moveLeftBtnIcon);
        m_fields->m_buttonMenu->addChild(m_fields->m_moveLeftBtn);

        // create the right btn
        auto rightBtnSprite = ButtonSprite::create("", 15, true, "bigFont.fnt", "GJ_button_01.png", 35.f, 0.1f);
        m_fields->m_moveRightBtnBg = rightBtnSprite; // Store reference

        m_fields->m_moveRightBtn = CCMenuItemSpriteExtra::create(
            rightBtnSprite,
            this,
            menu_selector(MyEditorUI::onMoveRightButton)
        );
        m_fields->m_moveRightBtn->setID("move-right");
        m_fields->m_moveRightBtn->ignoreAnchorPointForPosition(false);
        m_fields->m_moveRightBtn->setAnchorPoint({ 0.5, 0.5 });
        m_fields->m_moveRightBtn->setPosition({ (m_fields->m_buttonMenu->getContentWidth() / 2.f) + 35.f, m_fields->m_buttonMenu->getContentHeight() / 2.f });

        // move right button icon
        auto moveRightBtnIcon = CCSprite::createWithSpriteFrameName(moveBtnIconSpriteName.c_str());
        moveRightBtnIcon->setScale(iconScale);
        moveRightBtnIcon->setAnchorPoint({ 0.5, 0.5 });
        moveRightBtnIcon->ignoreAnchorPointForPosition(false);
        moveRightBtnIcon->setPosition({ m_fields->m_moveRightBtn->getContentWidth() / 2.f, m_fields->m_moveRightBtn->getContentHeight() / 2.f });
        moveRightBtnIcon->setRotation(90.f);

        m_fields->m_moveRightIcon = moveRightBtnIcon; // Store reference

        m_fields->m_moveRightBtn->addChild(moveRightBtnIcon);
        m_fields->m_buttonMenu->addChild(m_fields->m_moveRightBtn);

        // create the move size selector button (center) - draggable via touch handling
        auto sizeBtnSprite = CCSprite::createWithSpriteFrameName("gj_navDotBtn_off_001.png");

        m_fields->m_moveSizeBtn = CCMenuItemSpriteExtra::create(
            sizeBtnSprite,
            this,
            menu_selector(MyEditorUI::onMoveSizeButton)
        );
        m_fields->m_moveSizeBtn->setID("move-size");
        m_fields->m_moveSizeBtn->ignoreAnchorPointForPosition(false);
        m_fields->m_moveSizeBtn->setAnchorPoint({ 0.5, 0.5 });
        m_fields->m_moveSizeBtn->setPosition({ m_fields->m_buttonMenu->getContentWidth() / 2.f, m_fields->m_buttonMenu->getContentHeight() / 2.f });

        m_fields->m_buttonMenu->addChild(m_fields->m_moveSizeBtn);

        // create rotate counter clockwise button
        auto rotateCounterClockwiseSprite = ButtonSprite::create("", 18, true, "bigFont.fnt", "GJ_button_01.png", 33.5f, 0.1f);
        m_fields->m_rotateCounterClockwiseBtnBg = rotateCounterClockwiseSprite;

        m_fields->m_rotateCounterClockwiseBtn = CCMenuItemSpriteExtra::create(
            rotateCounterClockwiseSprite,
            this,
            menu_selector(MyEditorUI::onRotateCounterClockwiseButton)
        );
        m_fields->m_rotateCounterClockwiseBtn->setID("rotate-counter-clockwise");
        m_fields->m_rotateCounterClockwiseBtn->ignoreAnchorPointForPosition(false);
        m_fields->m_rotateCounterClockwiseBtn->setAnchorPoint({ 0.5, 0.5 });
        m_fields->m_rotateCounterClockwiseBtn->setPosition({ (m_fields->m_buttonMenu->getContentWidth() / 2.f) - 40.f, (m_fields->m_buttonMenu->getContentHeight() / 2.f) + 40.f });

        // rotate counter clockwise button icon
        auto rotateCounterClockwiseBtnIcon = CCSprite::createWithSpriteFrameName("edit_rotate45lBtn_001.png");
        rotateCounterClockwiseBtnIcon->setScale(iconScale);
        rotateCounterClockwiseBtnIcon->setAnchorPoint({ 0.5, 0.5 });
        rotateCounterClockwiseBtnIcon->ignoreAnchorPointForPosition(false);
        rotateCounterClockwiseBtnIcon->setPosition({ m_fields->m_rotateCounterClockwiseBtn->getContentWidth() / 2.f, m_fields->m_rotateCounterClockwiseBtn->getContentHeight() / 2.f });

        m_fields->m_rotateCounterClockwiseBtn->addChild(rotateCounterClockwiseBtnIcon);
        m_fields->m_buttonMenu->addChild(m_fields->m_rotateCounterClockwiseBtn);

        // create rotate clockwise button
        auto rotateClockwiseSprite = ButtonSprite::create("", 18, true, "bigFont.fnt", "GJ_button_01.png", 33.5f, 0.1f);
        m_fields->m_rotateClockwiseBtnBg = rotateClockwiseSprite;

        m_fields->m_rotateClockwiseBtn = CCMenuItemSpriteExtra::create(
            rotateClockwiseSprite,
            this,
            menu_selector(MyEditorUI::onRotateClockwiseButton) // will add function
        );
        m_fields->m_rotateClockwiseBtn->setID("rotate-clockwise");
        m_fields->m_rotateClockwiseBtn->ignoreAnchorPointForPosition(false);
        m_fields->m_rotateClockwiseBtn->setAnchorPoint({ 0.5, 0.5 });
        m_fields->m_rotateClockwiseBtn->setPosition({ (m_fields->m_buttonMenu->getContentWidth() / 2.f) + 40.f, (m_fields->m_buttonMenu->getContentHeight() / 2.f) + 40.f });

        // rotate clockwise button icon
        auto rotateClockwiseBtnIcon = CCSprite::createWithSpriteFrameName("edit_rotate45rBtn_001.png");
        rotateClockwiseBtnIcon->setScale(iconScale);
        rotateClockwiseBtnIcon->setAnchorPoint({ 0.5, 0.5 });
        rotateClockwiseBtnIcon->ignoreAnchorPointForPosition(false);
        rotateClockwiseBtnIcon->setPosition({ m_fields->m_rotateClockwiseBtn->getContentWidth() / 2.f, m_fields->m_rotateClockwiseBtn->getContentHeight() / 2.f });

        m_fields->m_rotateClockwiseBtn->addChild(rotateClockwiseBtnIcon);
        m_fields->m_buttonMenu->addChild(m_fields->m_rotateClockwiseBtn);

        // create flip x button
        auto flipXSprite = ButtonSprite::create("", 18, true, "bigFont.fnt", "GJ_button_01.png", 33.5f, 0.1f);
        m_fields->m_flipXBtnBg = flipXSprite;

        m_fields->m_flipXBtn = CCMenuItemSpriteExtra::create(
            flipXSprite,
            this,
            menu_selector(MyEditorUI::onFlipXButton) // will add function
        );
        m_fields->m_flipXBtn->setID("flip-x");
        m_fields->m_flipXBtn->ignoreAnchorPointForPosition(false);
        m_fields->m_flipXBtn->setAnchorPoint({ 0.5, 0.5 });
        m_fields->m_flipXBtn->setPosition({ (m_fields->m_buttonMenu->getContentWidth() / 2.f) - 40.f, (m_fields->m_buttonMenu->getContentHeight() / 2.f) - 40.f });

        // flip x button icon
        auto flipXBtnIcon = CCSprite::createWithSpriteFrameName("edit_flipXBtn_001.png");
        flipXBtnIcon->setScale(iconScale);
        flipXBtnIcon->setAnchorPoint({ 0.5, 0.5 });
        flipXBtnIcon->ignoreAnchorPointForPosition(false);
        flipXBtnIcon->setPosition({ m_fields->m_flipXBtn->getContentWidth() / 2.f, m_fields->m_flipXBtn->getContentHeight() / 2.f });

        m_fields->m_flipXBtn->addChild(flipXBtnIcon);
        m_fields->m_buttonMenu->addChild(m_fields->m_flipXBtn);

        // create flip y button
        auto flipYSprite = ButtonSprite::create("", 18, true, "bigFont.fnt", "GJ_button_01.png", 33.5f, 0.1f);
        m_fields->m_flipYBtnBg = flipYSprite;

        m_fields->m_flipYBtn = CCMenuItemSpriteExtra::create(
            flipYSprite,
            this,
            menu_selector(MyEditorUI::onFlipYButton) // will add function
        );
        m_fields->m_flipYBtn->setID("flip-y");
        m_fields->m_flipYBtn->ignoreAnchorPointForPosition(false);
        m_fields->m_flipYBtn->setAnchorPoint({ 0.5, 0.5 });
        m_fields->m_flipYBtn->setPosition({ (m_fields->m_buttonMenu->getContentWidth() / 2.f) + 40.f, (m_fields->m_buttonMenu->getContentHeight() / 2.f) - 40.f });

        // flip y button icon
        auto flipYBtnIcon = CCSprite::createWithSpriteFrameName("edit_flipYBtn_001.png");
        flipYBtnIcon->setScale(iconScale);
        flipYBtnIcon->setAnchorPoint({ 0.5, 0.5 });
        flipYBtnIcon->ignoreAnchorPointForPosition(false);
        flipYBtnIcon->setPosition({ m_fields->m_flipYBtn->getContentWidth() / 2.f, m_fields->m_flipYBtn->getContentHeight() / 2.f });

        m_fields->m_flipYBtn->addChild(flipYBtnIcon);
        m_fields->m_buttonMenu->addChild(m_fields->m_flipYBtn);

        // add the whole menu
        addChild(m_fields->m_buttonMenu);

        // Apply button background visibility setting
        updateButtonBackgroundVisibility();

        // Apply menu background visibility setting
        updateMenuBackgroundVisibility();

        // Apply menu size setting
        updateMenuSize();

        // Apply scale setting
        updateMenuScale();

        // Apply opacity setting
        updateMenuOpacity();

        // Initialize rotation buttons (will be updated when objects are selected)
        updateRotationButtons();

        // shocking it aint spagetti code :D
        // see cheese i made u proud with my amazing dumb coding skills

        // very proud :)

        return true; // yay
    };

    void showUI(bool show) {
        EditorUI::showUI(show);

        // Track UI visibility state
        m_fields->m_isUIVisible = show;

        // Update visibility when UI is shown/hidden
        if (show) {
            updateButtonMenuVisibility();
        } else {
            // Always hide the menu when UI is hidden, regardless of persistent setting
            if (m_fields->m_buttonMenu) m_fields->m_buttonMenu->setVisible(false);
        };
    };

    // move, rotate, and flip objects using editcommand
    void onMoveTransformCall(EditCommand editCommand, TransformType type) {
        if (!m_editorLayer) {
            log::error("No editor layer found");
            return;
        };

        // Check if there are objects selected
        if ((!m_selectedObjects || m_selectedObjects->count() == 0) && !m_selectedObject) {
            log::error("No objects selected to move");
            return;
        };

        log::info("Using edit command: {}", static_cast<int>(editCommand));

        // use move or transform depending on the kind of button being pressed
        switch (type) {
        case TransformType::Move:
            EditorUI::moveObjectCall(editCommand);
            break;

        default:
            EditorUI::transformObjectCall(editCommand);
            break;
        };
    };

    // hmmmm buttons my beloved

    void onMoveUpButton(CCObject*) {
        onMoveTransformCall(getEditCmd(m_fields->moveSize, MoveDirection::Up), TransformType::Move);
    };

    void onMoveDownButton(CCObject*) {
        onMoveTransformCall(getEditCmd(m_fields->moveSize, MoveDirection::Down), TransformType::Move);
    };

    void onMoveLeftButton(CCObject*) {
        onMoveTransformCall(getEditCmd(m_fields->moveSize, MoveDirection::Left), TransformType::Move);
    };

    void onMoveRightButton(CCObject*) {
        onMoveTransformCall(getEditCmd(m_fields->moveSize, MoveDirection::Right), TransformType::Move);
    };

    // i was gonna do if else statement but after learning about my c lessons in uni about switch statements, i use this :D

    // Cycle through move sizes
    void onMoveSizeButton(CCObject*) {
        switch (m_fields->moveSize) {
        case MoveSize::Tiny:
            m_fields->moveSize = MoveSize::Small;
            break;

        case MoveSize::Small:
            m_fields->moveSize = MoveSize::Half;
            break;

        case MoveSize::Half:
            m_fields->moveSize = MoveSize::Normal;
            break;

        case MoveSize::Normal:
            m_fields->moveSize = MoveSize::Big;
            break;

        case MoveSize::Big:
            m_fields->moveSize = MoveSize::Tiny;
            break;

        default:
            m_fields->moveSize = MoveSize::Small;
            break;
        };

        // Update arrow icons
        updateArrowIcons();

        // Update rotation buttons in case scale changed
        updateRotationButtons();

        log::info("Move size changed to: {}", getMoveSizeName(m_fields->moveSize));
    };

    // rotate buttons

    void onRotateCounterClockwiseButton(CCObject*) {
        bool hasSolid = hasSolidObjects();
        EditCommand rotateCmd = hasSolid ? EditCommand::RotateCCW : EditCommand::RotateCCW45;
        onMoveTransformCall(rotateCmd, TransformType::Rotate);
    };

    void onRotateClockwiseButton(CCObject*) {
        bool hasSolid = hasSolidObjects();
        EditCommand rotateCmd = hasSolid ? EditCommand::RotateCW : EditCommand::RotateCW45;
        onMoveTransformCall(rotateCmd, TransformType::Rotate);
    };

    // flip buttons

    void onFlipXButton(CCObject*) {
        onMoveTransformCall(EditCommand::FlipX, TransformType::Flip);
    };

    void onFlipYButton(CCObject*) {
        onMoveTransformCall(EditCommand::FlipY, TransformType::Flip);
    };

    // Check if the current selection contains any solid or slope objects
    bool hasSolidObjects() {
        // Check single selected object
        if (m_selectedObject && (m_selectedObject->m_objectType == GameObjectType::Solid ||
                                 m_selectedObject->m_objectType == GameObjectType::Slope)) {
            return true;
        };

        // Check multiple selected objects
        if (m_selectedObjects && m_selectedObjects->count() > 0) {
            for (auto* obj : CCArrayExt<GameObject*>(m_selectedObjects)) {
                if (obj && (obj->m_objectType == GameObjectType::Solid ||
                            obj->m_objectType == GameObjectType::Slope)) {
                    return true;
                };
            };
        };

        return false;
    };

    // Update rotation button icons based on selection content
    void updateRotationButtons() {
        if (!m_fields->m_rotateClockwiseBtn || !m_fields->m_rotateCounterClockwiseBtn) return;

        bool hasSolid = hasSolidObjects();

        // Rotation icons should always use default scale (1.0f), not scale with move size
        float rotationIconScale = 1.0f;

        // Get the existing icons - they should be the last child added to each button
        CCSprite* clockwiseIcon = nullptr;
        CCSprite* counterClockwiseIcon = nullptr;

        // Find the icon sprites in the buttons
        auto clockwiseChildren = m_fields->m_rotateClockwiseBtn->getChildren();
        auto counterClockwiseChildren = m_fields->m_rotateCounterClockwiseBtn->getChildren();

        if (clockwiseChildren && clockwiseChildren->count() > 0) {
            // The icon should be the last child (added after the button sprite)
            clockwiseIcon = dynamic_cast<CCSprite*>(clockwiseChildren->lastObject());
        };

        if (counterClockwiseChildren && counterClockwiseChildren->count() > 0) {
            // The icon should be the last child (added after the button sprite)
            counterClockwiseIcon = dynamic_cast<CCSprite*>(counterClockwiseChildren->lastObject());
        };

        if (clockwiseIcon && counterClockwiseIcon) {
            // Choose appropriate sprite frames
            const char* clockwiseSprite = hasSolid ? "edit_cwBtn_001.png" : "edit_rotate45rBtn_001.png";
            const char* counterClockwiseSprite = hasSolid ? "edit_ccwBtn_001.png" : "edit_rotate45lBtn_001.png";

            // Update sprites
            auto clockwiseFrame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(clockwiseSprite);
            auto counterClockwiseFrame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(counterClockwiseSprite);

            if (clockwiseFrame) {
                clockwiseIcon->setDisplayFrame(clockwiseFrame);
                clockwiseIcon->setScale(rotationIconScale);
            };

            if (counterClockwiseFrame) {
                counterClockwiseIcon->setDisplayFrame(counterClockwiseFrame);
                counterClockwiseIcon->setScale(rotationIconScale);
            };
        };
    };

    void updateButtonMenuVisibility() {
        if (!m_fields->m_buttonMenu) return;

        bool hasSelection = false;

        // Check if there are any selected objects
        if (m_selectedObjects && m_selectedObjects->count() > 0) {
            hasSelection = true;
        } else if (m_selectedObject) {
            hasSelection = true;
        };

        // Check if persistent button setting is enabled
        bool isPersistent = qmbMod->getSettingValue<bool>("presistent-btn");

        // Use tracked UI visibility state instead of isVisible()
        bool isUIVisible = m_fields->m_isUIVisible;

        // Determine if menu should be visible (either has selection OR persistent mode is enabled) AND UI is visible
        bool shouldBeVisible = (hasSelection || isPersistent) && isUIVisible;

        // Only log when visibility actually changes
        bool currentVisibility = m_fields->m_buttonMenu->isVisible();
        if (currentVisibility != shouldBeVisible) m_fields->m_buttonMenu->setVisible(shouldBeVisible);

        // Update rotation buttons when selection changes or when persistent mode shows menu
        if (hasSelection) {
            updateRotationButtons();
        } else if (isPersistent && isUIVisible) {
            // When persistent but no selection, still update rotation buttons for consistency (only if UI is visible)
            updateRotationButtons();
        };
    };

    void updateButtonBackgroundVisibility() {
        bool showButtonBG = qmbMod->getSettingValue<bool>("visible-bg");

        // good idea to use the buttonsprite, way better than using CCMenuItemSpriteExtra :)
        // also if statement wall
        if (m_fields->m_moveUpBtnBg) m_fields->m_moveUpBtnBg->setVisible(showButtonBG);
        if (m_fields->m_moveDownBtnBg) m_fields->m_moveDownBtnBg->setVisible(showButtonBG);
        if (m_fields->m_moveLeftBtnBg) m_fields->m_moveLeftBtnBg->setVisible(showButtonBG);
        if (m_fields->m_moveRightBtnBg) m_fields->m_moveRightBtnBg->setVisible(showButtonBG);

        if (m_fields->m_rotateCounterClockwiseBtnBg) m_fields->m_rotateCounterClockwiseBtnBg->setVisible(showButtonBG);
        if (m_fields->m_rotateClockwiseBtnBg) m_fields->m_rotateClockwiseBtnBg->setVisible(showButtonBG);

        if (m_fields->m_flipXBtnBg) m_fields->m_flipXBtnBg->setVisible(showButtonBG);
        if (m_fields->m_flipYBtnBg) m_fields->m_flipYBtnBg->setVisible(showButtonBG);
    };

    void updateMenuScale() {
        if (!m_fields->m_buttonMenu) return;

        // Get the scale value from settings
        float scaleValue = static_cast<float>(qmbMod->getSettingValue<double>("scale-btns"));

        // Apply the scale to the entire button menu
        m_fields->m_buttonMenu->setScale(scaleValue);
    };

    // Override methods that handle object selection to update visibility
    void selectObject(GameObject * obj, bool filter) {
        EditorUI::selectObject(obj, filter);
        updateButtonMenuVisibility();
    };

    void selectObjects(CCArray * objs, bool ignoreFilters) {
        EditorUI::selectObjects(objs, ignoreFilters);
        updateButtonMenuVisibility();
    };

    void deselectAll() {
        EditorUI::deselectAll();
        updateButtonMenuVisibility();
    };

    void deselectObject(GameObject * obj) {
        EditorUI::deselectObject(obj);
        updateButtonMenuVisibility();
    };

    void update(float dt) {
        EditorUI::update(dt);

        // Very frequent check for better paste detection - check every frame but throttle updates
        static float timer = 0.0f;
        static bool lastHasSelection = false;

        timer += dt;

        // Check selection state every frame
        bool currentHasSelection = false;
        if (m_selectedObjects && m_selectedObjects->count() > 0) {
            currentHasSelection = true;
        } else if (m_selectedObject) {
            currentHasSelection = true;
        };

        // If selection state changed, update immediately
        if (currentHasSelection != lastHasSelection) {
            updateButtonMenuVisibility();
            lastHasSelection = currentHasSelection;

            timer = 0.0f; // Reset timer
        };

        // Also do periodic updates for settings changes
        if (timer >= 0.2f) { // Check settings every 0.2 seconds
            updateButtonBackgroundVisibility(); // Check setting changes
            updateMenuBackgroundVisibility(); // Check menu background setting changes
            updateMenuSize(); // Check menu size setting changes
            updateMenuScale(); // Check scale setting changes
            updateMenuOpacity(); // Check opacity setting changes

            timer = 0.0f;
        };
    };

    // hook is not currently bound on these platforms
#if !defined(GEODE_IS_MACOS) && !defined(GEODE_IS_IOS)

    void triggerSwipeMode() {
        // Only trigger if not already touching the menu
        if (!m_fields->m_dontSwipe) EditorUI::triggerSwipeMode();
    };

#endif

    // Override mouse/touch events to catch selection changes and handle dragging
    bool ccTouchBegan(CCTouch * touch, CCEvent * event) {
        // Reset dragging state
        m_fields->m_isDragging = false;
        m_fields->m_touchStartPos = CCPoint(0, 0);

        // Check if touch is on the menu background area (but not on any buttons)
        bool touchOnMenuBackground = false;
        if (m_fields->m_buttonMenu && m_fields->m_buttonMenu->isVisible()) {
            // Convert touch to the menu's coordinate space
            CCPoint menuLocal = m_fields->m_buttonMenu->convertTouchToNodeSpace(touch);
            CCSize menuSize = m_fields->m_buttonMenu->getContentSize();

            // Check if touch is within the menu bounds
            CCRect menuRect = CCRect(0, 0, menuSize.width, menuSize.height);

            if (menuRect.containsPoint(menuLocal)) {
                // Now check if it's NOT on any of the buttons
                bool onButton = false;

                // Disable accidental swiping
                m_fields->m_dontSwipe = true;

                // Check all directional buttons
                if (m_fields->m_moveUpBtn && m_fields->m_moveUpBtn->isVisible()) {
                    CCPoint btnPos = m_fields->m_moveUpBtn->getPosition();
                    CCSize btnSize = m_fields->m_moveUpBtn->getContentSize();
                    CCRect btnRect = CCRect(btnPos.x - btnSize.width / 2, btnPos.y - btnSize.height / 2, btnSize.width, btnSize.height);

                    if (btnRect.containsPoint(menuLocal)) onButton = true;
                };

                if (!onButton && m_fields->m_moveDownBtn && m_fields->m_moveDownBtn->isVisible()) {
                    CCPoint btnPos = m_fields->m_moveDownBtn->getPosition();
                    CCSize btnSize = m_fields->m_moveDownBtn->getContentSize();
                    CCRect btnRect = CCRect(btnPos.x - btnSize.width / 2, btnPos.y - btnSize.height / 2, btnSize.width, btnSize.height);

                    if (btnRect.containsPoint(menuLocal)) onButton = true;
                };

                if (!onButton && m_fields->m_moveLeftBtn && m_fields->m_moveLeftBtn->isVisible()) {
                    CCPoint btnPos = m_fields->m_moveLeftBtn->getPosition();
                    CCSize btnSize = m_fields->m_moveLeftBtn->getContentSize();
                    CCRect btnRect = CCRect(btnPos.x - btnSize.width / 2, btnPos.y - btnSize.height / 2, btnSize.width, btnSize.height);

                    if (btnRect.containsPoint(menuLocal)) onButton = true;
                };

                if (!onButton && m_fields->m_moveRightBtn && m_fields->m_moveRightBtn->isVisible()) {
                    CCPoint btnPos = m_fields->m_moveRightBtn->getPosition();
                    CCSize btnSize = m_fields->m_moveRightBtn->getContentSize();
                    CCRect btnRect = CCRect(btnPos.x - btnSize.width / 2, btnPos.y - btnSize.height / 2, btnSize.width, btnSize.height);

                    if (btnRect.containsPoint(menuLocal)) onButton = true;
                };

                if (!onButton && m_fields->m_moveSizeBtn && m_fields->m_moveSizeBtn->isVisible()) {
                    CCPoint btnPos = m_fields->m_moveSizeBtn->getPosition();
                    CCSize btnSize = m_fields->m_moveSizeBtn->getContentSize();
                    CCRect btnRect = CCRect(btnPos.x - btnSize.width / 2, btnPos.y - btnSize.height / 2, btnSize.width, btnSize.height);

                    if (btnRect.containsPoint(menuLocal)) onButton = true;
                };

                // If we're in the menu area but not on any button, we can start dragging (if enabled)
                if (!onButton) {
                    bool noDragging = qmbMod->getSettingValue<bool>("no-dragging");

                    if (!noDragging) {
                        touchOnMenuBackground = true;

                        m_fields->m_touchStartPos = touch->getLocation();
                        m_fields->m_menuStartPos = m_fields->m_buttonMenu->getPosition();

                        log::info("Touch began on menu background at ({}, {}), ready for drag detection", m_fields->m_touchStartPos.x, m_fields->m_touchStartPos.y);
                    };
                };
            };
        };

        // Call parent for normal touch handling
        bool result = EditorUI::ccTouchBegan(touch, event);

        // Delay the visibility update slightly to let selection complete
        runAction(CCSequence::create(
            CCDelayTime::create(0.01f),
            CCCallFunc::create(this, callfunc_selector(MyEditorUI::updateButtonMenuVisibility)),
            nullptr
        ));

        return result;
    };

    void ccTouchMoved(CCTouch * touch, CCEvent * event) {
        // Handle dragging if we started on the menu background
        if (m_fields->m_touchStartPos.x != 0 || m_fields->m_touchStartPos.y != 0) {
            CCPoint currentPos = touch->getLocation();
            float distance = ccpDistance(currentPos, m_fields->m_touchStartPos);

            // Start dragging if moved more than 5 pixels (responsive)
            if (!m_fields->m_isDragging && distance > 5.0f) {
                m_fields->m_isDragging = true;
                log::info("Started dragging menu from background - distance: {}", distance);

                // Update opacity when dragging starts
                updateMenuOpacity();
            };

            if (m_fields->m_isDragging) {
                CCPoint delta = ccpSub(currentPos, m_fields->m_touchStartPos);
                CCPoint newPos = ccpAdd(m_fields->m_menuStartPos, delta);

                // Constrain position to screen bounds
                auto winSize = CCDirector::sharedDirector()->getWinSize();

                float menuHalfWidth = 65.0f; // Account for scaled buttons
                float menuHalfHeight = 65.0f;

                newPos.x = std::max(menuHalfWidth, std::min(winSize.width - menuHalfWidth, newPos.x));
                newPos.y = std::max(menuHalfHeight, std::min(winSize.height - menuHalfHeight, newPos.y));

                m_fields->m_buttonMenu->setPosition(newPos);
                return; // Don't call parent if we're dragging
            };
        };

        EditorUI::ccTouchMoved(touch, event);
    };

    void ccTouchEnded(CCTouch * touch, CCEvent * event) {
        bool wasDragging = m_fields->m_isDragging;
        bool hadTouchStart = (m_fields->m_touchStartPos.x != 0 || m_fields->m_touchStartPos.y != 0);

        // Allow swipe mode again
        m_fields->m_dontSwipe = false;

        if (m_fields->m_isDragging) {
            // Save the new position
            CCPoint pos = m_fields->m_buttonMenu->getPosition();

            qmbMod->setSavedValue("menuPositionX", pos.x);
            qmbMod->setSavedValue("menuPositionY", pos.y);

            log::info("Saved new menu position: ({}, {})", pos.x, pos.y);
            m_fields->m_isDragging = false;

            // Restore opacity after dragging
            updateMenuOpacity();
        };

        // Reset touch tracking
        m_fields->m_touchStartPos = CCPoint(0, 0);

        // Always call parent to ensure proper cleanup
        EditorUI::ccTouchEnded(touch, event);
    };

    void updateArrowIcons() {
        if (!m_fields->m_moveUpIcon || !m_fields->m_moveDownIcon ||
            !m_fields->m_moveLeftIcon || !m_fields->m_moveRightIcon) return;

        auto const iconInfo = getMoveSizeIconInfo(m_fields->moveSize);
        auto const newSpriteName = iconInfo.first;
        auto newScale = iconInfo.second;

        // Update up arrow
        auto newFrame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(newSpriteName.c_str());
        if (newFrame) {
            m_fields->m_moveUpIcon->setDisplayFrame(newFrame);
            m_fields->m_moveUpIcon->setScale(newScale);
        };

        // Update down arrow
        if (newFrame) {
            m_fields->m_moveDownIcon->setDisplayFrame(newFrame);
            m_fields->m_moveDownIcon->setScale(newScale);
            m_fields->m_moveDownIcon->setFlipY(true); // Keep flipped
        };

        // Update left arrow
        if (newFrame) {
            m_fields->m_moveLeftIcon->setDisplayFrame(newFrame);
            m_fields->m_moveLeftIcon->setScale(newScale);
            m_fields->m_moveLeftIcon->setRotation(-90.f); // Keep rotated
        };

        // Update right arrow
        if (newFrame) {
            m_fields->m_moveRightIcon->setDisplayFrame(newFrame);
            m_fields->m_moveRightIcon->setScale(newScale);
            m_fields->m_moveRightIcon->setRotation(90.f); // Keep rotated
        };
    };

    // Create button sprite based on settings
    ButtonSprite* createMoveButtonSprite(const std::string & text = "", int width = 20, bool absolute = true, const std::string & font = "bigFont.fnt", int height = 30, float scale = 0.1f) {
        bool showButtonBG = qmbMod->getSettingValue<bool>("visible-bg");

        if (showButtonBG) {
            return ButtonSprite::create(text.c_str(), width, absolute, font.c_str(), "GJ_button_01.png", height, scale);
        } else {
            // Create button without background - use transparent sprite or no background sprite
            return ButtonSprite::create(text.c_str(), width, absolute, font.c_str(), "square02_small.png", height, scale);
        };
    };

    void updateMenuOpacity() {
        if (!m_fields->m_buttonMenu) return;

        // Get the opacity value from settings
        int baseOpacity = static_cast<int>(qmbMod->getSettingValue<int64_t>("opacity-btn"));

        // If dragging, reduce opacity by 50
        int currentOpacity = m_fields->m_isDragging ? std::max(0, baseOpacity - 50) : baseOpacity;

        // Apply the opacity to the entire button menu
        m_fields->m_buttonMenu->setOpacity(currentOpacity);

        // Also update the menu background opacity separately if they exist and are visible
        if (m_fields->m_buttonMenuBg && m_fields->m_buttonMenuBg->isVisible()) {
            // Base opacity for main menu background is 50, reduce to 25 when dragging
            int menuBgOpacity = m_fields->m_isDragging ? 25 : 50;
            m_fields->m_buttonMenuBg->setOpacity(menuBgOpacity);
        };

        if (m_fields->m_buttonMenuBgDepth && m_fields->m_buttonMenuBgDepth->isVisible()) {
            // Base opacity for depth background is 25, reduce to 15 when dragging
            int depthBgOpacity = m_fields->m_isDragging ? 15 : 25;
            m_fields->m_buttonMenuBgDepth->setOpacity(depthBgOpacity);
        };
    };

    void updateMenuBackgroundVisibility() {
        if (!m_fields->m_buttonMenuBg || !m_fields->m_buttonMenuBgDepth) return;

        bool showMenuBG = qmbMod->getSettingValue<bool>("menu-btn");

        // Update visibility for both backgrounds
        m_fields->m_buttonMenuBg->setVisible(showMenuBG);
        m_fields->m_buttonMenuBgDepth->setVisible(showMenuBG);

        // Set proper opacity when visibility changes
        if (showMenuBG) {
            // Main background opacity
            int menuBgOpacity = m_fields->m_isDragging ? 25 : 50;
            m_fields->m_buttonMenuBg->setOpacity(menuBgOpacity);

            // Depth background opacity (always lower for shadow effect)
            int depthBgOpacity = m_fields->m_isDragging ? 15 : 25;
            m_fields->m_buttonMenuBgDepth->setOpacity(depthBgOpacity);
        };
    };

    void repositionButtons() {
        if (!m_fields->m_buttonMenu) return;

        float centerX = m_fields->m_buttonMenu->getContentWidth() / 2.f;
        float centerY = m_fields->m_buttonMenu->getContentHeight() / 2.f;

        // Reposition all buttons relative to the center
        if (m_fields->m_moveUpBtn) m_fields->m_moveUpBtn->setPosition({ centerX, centerY + 35.f });
        if (m_fields->m_moveDownBtn) m_fields->m_moveDownBtn->setPosition({ centerX, centerY - 35.f });
        if (m_fields->m_moveLeftBtn) m_fields->m_moveLeftBtn->setPosition({ centerX - 35.f, centerY });
        if (m_fields->m_moveRightBtn) m_fields->m_moveRightBtn->setPosition({ centerX + 35.f, centerY });

        if (m_fields->m_moveSizeBtn) m_fields->m_moveSizeBtn->setPosition({ centerX, centerY });

        if (m_fields->m_rotateCounterClockwiseBtn) m_fields->m_rotateCounterClockwiseBtn->setPosition({ centerX - 40.f, centerY + 40.f });
        if (m_fields->m_rotateClockwiseBtn) m_fields->m_rotateClockwiseBtn->setPosition({ centerX + 40.f, centerY + 40.f });

        if (m_fields->m_flipXBtn) m_fields->m_flipXBtn->setPosition({ centerX - 40.f, centerY - 40.f });
        if (m_fields->m_flipYBtn) m_fields->m_flipYBtn->setPosition({ centerX + 40.f, centerY - 40.f });
    };

    void updateMenuSize() {
        if (!m_fields->m_buttonMenu) return;

        // Get background size from settings
        int bgSize = static_cast<int>(qmbMod->getSettingValue<int64_t>("scale-bg"));

        CCSize const newSize = { static_cast<float>(bgSize), static_cast<float>(bgSize) };
        CCSize const depthSize = { static_cast<float>(bgSize + 6), static_cast<float>(bgSize + 6) }; // Larger for depth

        // Update menu content size
        m_fields->m_buttonMenu->setContentSize(newSize);

        // Reposition all buttons to match the new center
        repositionButtons();

        // Update main background size and position to keep it centered
        if (m_fields->m_buttonMenuBg) {
            m_fields->m_buttonMenuBg->setContentSize(newSize);
            m_fields->m_buttonMenuBg->setPosition({ m_fields->m_buttonMenu->getContentWidth() / 2.f, m_fields->m_buttonMenu->getContentHeight() / 2.f });
        };

        // Update depth background size and position (larger but same center position)
        if (m_fields->m_buttonMenuBgDepth) {
            m_fields->m_buttonMenuBgDepth->setContentSize(depthSize);
            m_fields->m_buttonMenuBgDepth->setPosition({ m_fields->m_buttonMenu->getContentWidth() / 2.f, m_fields->m_buttonMenu->getContentHeight() / 2.f });
        };
    };
};

class $modify(MyEditorPauseLayer, EditorPauseLayer) {
    bool init(LevelEditorLayer * p0) {
        if (!EditorPauseLayer::init(p0)) return false;

        auto guidelinesMenu = getChildByID("guidelines-menu");

        auto modSettingsBtnSprite = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
        modSettingsBtnSprite->setScale(0.875f);

        auto modSettingsBtnSpriteIcon = CCSprite::createWithSpriteFrameName("edit_areaModeBtn04_001.png");
        modSettingsBtnSpriteIcon->ignoreAnchorPointForPosition(false);
        modSettingsBtnSpriteIcon->setAnchorPoint({ 0.5, 0.5 });
        modSettingsBtnSpriteIcon->setPosition({ (modSettingsBtnSprite->getScaledContentWidth() / 2.f) + 2.5f, (modSettingsBtnSprite->getScaledContentHeight() / 2.f) + 2.5f });
        modSettingsBtnSpriteIcon->setScale(2.f);

        modSettingsBtnSprite->addChild(modSettingsBtnSpriteIcon);

        auto modSettingsBtn = CCMenuItemSpriteExtra::create(
            modSettingsBtnSprite,
            this,
            menu_selector(MyEditorPauseLayer::onQmbModSettings)
        );

        guidelinesMenu->addChild(modSettingsBtn);
        guidelinesMenu->updateLayout(true);

        return true;
    };

    void onQmbModSettings(CCObject*) {
        openSettingsPopup(qmbMod);
    };
};