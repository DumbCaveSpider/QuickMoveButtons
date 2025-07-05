#include "./QuickMove.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/utils/NodeIDs.hpp>

using namespace geode::prelude;
using namespace quickmove;

// Returns the EditCommand based on object move size and direction
EditCommand getEditCmd(MoveSize moveSize, MoveDirection moveDir) {
    switch (moveSize) {
    case MoveSize::Tiny:
        switch (moveDir) {
        case MoveDirection::Up:    return EditCommand::TinyUp;
        case MoveDirection::Down:  return EditCommand::TinyDown;
        case MoveDirection::Left:  return EditCommand::TinyLeft;
        case MoveDirection::Right: return EditCommand::TinyRight;
        };
        break;

    case MoveSize::Small:
        switch (moveDir) {
        case MoveDirection::Up:    return EditCommand::SmallUp;
        case MoveDirection::Down:  return EditCommand::SmallDown;
        case MoveDirection::Left:  return EditCommand::SmallLeft;
        case MoveDirection::Right: return EditCommand::SmallRight;
        };
        break;

    case MoveSize::Half:
        switch (moveDir) {
        case MoveDirection::Up:    return EditCommand::HalfUp;
        case MoveDirection::Down:  return EditCommand::HalfDown;
        case MoveDirection::Left:  return EditCommand::HalfLeft;
        case MoveDirection::Right: return EditCommand::HalfRight;
        };
        break;

    case MoveSize::Normal:
        switch (moveDir) {
        case MoveDirection::Up:    return EditCommand::Up;
        case MoveDirection::Down:  return EditCommand::Down;
        case MoveDirection::Left:  return EditCommand::Left;
        case MoveDirection::Right: return EditCommand::Right;
        };
        break;

    case MoveSize::Big:
        switch (moveDir) {
        case MoveDirection::Up:    return EditCommand::BigUp;
        case MoveDirection::Down:  return EditCommand::BigDown;
        case MoveDirection::Left:  return EditCommand::BigLeft;
        case MoveDirection::Right: return EditCommand::BigRight;
        };
        break;

    default:
        return EditCommand::SmallUp;
    };

    return EditCommand::SmallUp;
};

// Get the string name for a move size
std::string getMoveSizeName(MoveSize moveSize) {
    switch (moveSize) {
    case MoveSize::Tiny:
        return "Tiny";

    case MoveSize::Small:
        return "Small";

    case MoveSize::Half:
        return "Half";

    case MoveSize::Normal:
        return "Normal";

    case MoveSize::Big:
        return "Big";

    default:
        return "Small";
    };
};

// Get the sprite name and scale for arrow icons based on move size
std::pair<std::string, float> getMoveSizeIconInfo(MoveSize moveSize) {
    float defScale = 0.875f;

    switch (moveSize) {
    case MoveSize::Tiny:
        return { "edit_upBtn_001.png", 0.625f };

    case MoveSize::Small:
        return { "edit_upBtn_001.png", defScale };

    case MoveSize::Half:
        return { "edit_upBtn5_001.png", defScale };

    case MoveSize::Normal:
        return { "edit_upBtn2_001.png", defScale };

    case MoveSize::Big:
        return { "edit_upBtn3_001.png", defScale };

    default:
        return { "edit_upBtn_001.png", defScale };
    };
};

class $modify(MyEditorUI, EditorUI) {
    struct Fields {
        MoveSize moveSize = MoveSize::Small;

        CCMenu* m_buttonMenu;
        CCScale9Sprite* m_buttonMenuBg;

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

        ButtonSprite* m_moveSizeBtnBg;

        ButtonSprite* m_rotateClockwiseBtnBg;
        ButtonSprite* m_rotateCounterClockwiseBtnBg;

        ButtonSprite* m_flipXBtnBg;
        ButtonSprite* m_flipYBtnBg;

        // Dragging functionality
        bool m_isDragging = false;

        CCPoint m_touchStartPos;
        CCPoint m_menuStartPos;
    };

    bool init(LevelEditorLayer * p0) {
        if (!EditorUI::init(p0)) return false;

        // create menu for the buttons
        m_fields->m_buttonMenu = CCMenu::create();
        m_fields->m_buttonMenu->setID("quick-move-menu"_spr);

        // Get saved position or use center of screen as default
        float savedX = Mod::get()->getSavedValue<float>("menuPositionX", CCDirector::sharedDirector()->getWinSize().width / 2);
        float savedY = Mod::get()->getSavedValue<float>("menuPositionY", CCDirector::sharedDirector()->getWinSize().height / 2);

        m_fields->m_buttonMenu->setPosition(CCPoint(savedX, savedY));
        m_fields->m_buttonMenu->setAnchorPoint({ 0.5f, 0.5f });
        m_fields->m_buttonMenu->setContentSize({ 125.0f, 125.0f });
        m_fields->m_buttonMenu->ignoreAnchorPointForPosition(false);
        m_fields->m_buttonMenu->setVisible(false); // Initially invisible since no objects are selected

        // create visible background for the menu
        auto buttonMenuBg = CCScale9Sprite::create("square02_001.png");
        buttonMenuBg->setContentSize(m_fields->m_buttonMenu->getContentSize());
        buttonMenuBg->ignoreAnchorPointForPosition(false);
        buttonMenuBg->setAnchorPoint({ 0.5, 0.5 });
        buttonMenuBg->setOpacity(100); // Slightly more visible for dragging
        buttonMenuBg->setPosition({ m_fields->m_buttonMenu->getContentWidth() / 2.f, m_fields->m_buttonMenu->getContentHeight() / 2.f });
        buttonMenuBg->setVisible(false); // Always invisible

        m_fields->m_buttonMenuBg = buttonMenuBg;
        m_fields->m_buttonMenu->addChild(buttonMenuBg);

        // sprite for all buttons
        // Get current move size icon info
        auto iconInfo = getMoveSizeIconInfo(m_fields->moveSize);
        auto moveBtnIconSpriteName = iconInfo.first;
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
        m_fields->m_moveUpBtn->setPosition({ m_fields->m_buttonMenu->getContentWidth() / 2.f, (m_fields->m_buttonMenu->getContentHeight() / 2.f) + 30.f });

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
        m_fields->m_moveDownBtn->setPosition({ m_fields->m_buttonMenu->getContentWidth() / 2.f, (m_fields->m_buttonMenu->getContentHeight() / 2.f) - 30.f });

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
        m_fields->m_moveSizeBtnBg = nullptr; // No ButtonSprite background for simple sprite

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
        auto rotateCounterClockwiseSprite = ButtonSprite::create("", 15, true, "bigFont.fnt", "GJ_button_01.png", 32.5f, 0.1f);
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
        auto rotateClockwiseSprite = ButtonSprite::create("", 15, true, "bigFont.fnt", "GJ_button_01.png", 32.5f, 0.1f);
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
        auto flipXSprite = ButtonSprite::create("", 15, true, "bigFont.fnt", "GJ_button_01.png", 32.5f, 0.1f);
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
        auto flipYSprite = ButtonSprite::create("", 15, true, "bigFont.fnt", "GJ_button_01.png", 32.5f, 0.1f);
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
        this->addChild(m_fields->m_buttonMenu);

        // Apply button background visibility setting
        updateButtonBackgroundVisibility();

        // Apply scale setting
        updateMenuScale();

        // Apply opacity setting
        updateMenuOpacity();

        // Initialize rotation buttons (will be updated when objects are selected)
        updateRotationButtons();

        return true;

        // shocking it aint spagetti code :D
        // see cheese i made u proud with my amazing dumb coding skills

        // very proud :)
    };

    void showUI(bool show) {
        EditorUI::showUI(show);

        // Update visibility when UI is shown/hidden (took this from better edit or something)
        if (show) {
            updateButtonMenuVisibility();
        } else {
            // Hide the menu when UI is hidden
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
            this->moveObjectCall(editCommand);
            break;

        default:
            this->transformObjectCall(editCommand);
            break;
        };
    };

    // hmmmm buttons my beloved

    void onMoveUpButton(CCObject*) {
        this->onMoveTransformCall(getEditCmd(m_fields->moveSize, MoveDirection::Up), TransformType::Move);
    };

    void onMoveDownButton(CCObject*) {
        this->onMoveTransformCall(getEditCmd(m_fields->moveSize, MoveDirection::Down), TransformType::Move);
    };

    void onMoveLeftButton(CCObject*) {
        this->onMoveTransformCall(getEditCmd(m_fields->moveSize, MoveDirection::Left), TransformType::Move);
    };

    void onMoveRightButton(CCObject*) {
        this->onMoveTransformCall(getEditCmd(m_fields->moveSize, MoveDirection::Right), TransformType::Move);
    };

    // i was gonna do if else statement but after learning about my c lessons in uni about switch statements, i use this :D

    void onMoveSizeButton(CCObject*) {
        // Cycle through move sizes
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
        this->onMoveTransformCall(rotateCmd, TransformType::Rotate);
    };

    void onRotateClockwiseButton(CCObject*) {
        bool hasSolid = hasSolidObjects();
        EditCommand rotateCmd = hasSolid ? EditCommand::RotateCW : EditCommand::RotateCW45;
        this->onMoveTransformCall(rotateCmd, TransformType::Rotate);
    };

    // flip buttons

    void onFlipXButton(CCObject*) {
        this->onMoveTransformCall(EditCommand::FlipX, TransformType::Flip);
    };

    void onFlipYButton(CCObject*) {
        this->onMoveTransformCall(EditCommand::FlipY, TransformType::Flip);
    };

    // Check if the current selection contains any solid or slope objects
    bool hasSolidObjects() {
        // Check single selected object
        if (m_selectedObject && (m_selectedObject->m_objectType == GameObjectType::Solid || 
                                m_selectedObject->m_objectType == GameObjectType::Slope)) {
            return true;
        }

        // Check multiple selected objects
        if (m_selectedObjects && m_selectedObjects->count() > 0) {
            for (auto* obj : CCArrayExt<GameObject*>(m_selectedObjects)) {
                if (obj && (obj->m_objectType == GameObjectType::Solid || 
                           obj->m_objectType == GameObjectType::Slope)) {
                    return true;
                }
            }
        }

        return false;
    }

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
        }

        if (counterClockwiseChildren && counterClockwiseChildren->count() > 0) {
            // The icon should be the last child (added after the button sprite)
            counterClockwiseIcon = dynamic_cast<CCSprite*>(counterClockwiseChildren->lastObject());
        }

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
            }

            if (counterClockwiseFrame) {
                counterClockwiseIcon->setDisplayFrame(counterClockwiseFrame);
                counterClockwiseIcon->setScale(rotationIconScale);
            }
        }
    }

    void updateButtonMenuVisibility() {
        if (!m_fields->m_buttonMenu) return;

        bool hasSelection = false;

        // Check if there are any selected objects
        if (m_selectedObjects && m_selectedObjects->count() > 0) {
            hasSelection = true;
        } else if (m_selectedObject) {
            hasSelection = true;
        };

        // Only log when visibility actually changes
        bool currentVisibility = m_fields->m_buttonMenu->isVisible();
        if (currentVisibility != hasSelection) {
            m_fields->m_buttonMenu->setVisible(hasSelection);
        };

        // Update rotation buttons when selection changes
        if (hasSelection) {
            updateRotationButtons();
        }
    };

    void updateButtonBackgroundVisibility() {
        bool showButtonBG = Mod::get()->getSettingValue<bool>("visible-bg");

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
        // Note: m_moveSizeBtnBg is now nullptr since we use a simple sprite instead of ButtonSprite
    };

    void updateMenuScale() {
        if (!m_fields->m_buttonMenu) return;

        // Get the scale value from settings
        float scaleValue = Mod::get()->getSettingValue<float>("scale-btn");

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

        // Periodically check selection state and settings
        static float timer = 0.0f;
        timer += dt;

        if (timer >= 0.5f) { // Check every 0.5 seconds
            updateButtonMenuVisibility();
            updateButtonBackgroundVisibility(); // Check setting changes
            updateMenuScale(); // Check scale setting changes
            updateMenuOpacity(); // Check opacity setting changes
            timer = 0.0f;
        };
    };

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

                // If we're in the menu area but not on any button, we can start dragging
                if (!onButton) {
                    touchOnMenuBackground = true;
                    m_fields->m_touchStartPos = touch->getLocation();
                    m_fields->m_menuStartPos = m_fields->m_buttonMenu->getPosition();
                    log::info("Touch began on menu background at ({}, {}), ready for drag detection", m_fields->m_touchStartPos.x, m_fields->m_touchStartPos.y);
                };
            };
        };

        // Call parent for normal touch handling
        bool result = EditorUI::ccTouchBegan(touch, event);

        // Delay the visibility update slightly to let selection complete
        this->runAction(CCSequence::create(
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

        if (m_fields->m_isDragging) {
            // Save the new position
            CCPoint pos = m_fields->m_buttonMenu->getPosition();

            Mod::get()->setSavedValue("menuPositionX", pos.x);
            Mod::get()->setSavedValue("menuPositionY", pos.y);

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

        auto iconInfo = getMoveSizeIconInfo(m_fields->moveSize);
        auto newSpriteName = iconInfo.first;
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
        bool showButtonBG = Mod::get()->getSettingValue<bool>("visible-bg");

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
        int baseOpacity = Mod::get()->getSettingValue<int>("opacityBtn");
        
        // If dragging, reduce opacity by 30
        int currentOpacity = m_fields->m_isDragging ? std::max(0, baseOpacity - 30) : baseOpacity;
        
        // Apply the opacity to the entire button menu
        m_fields->m_buttonMenu->setOpacity(currentOpacity);
    };
};