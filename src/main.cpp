#include "./QuickMove.hpp"

#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/utils/NodeIDs.hpp>

using namespace geode::prelude;
using namespace quickmove;

// Get the move offset float depending on the set move distance
float getMoveOffset(MoveSize moveSize) {
    switch (moveSize) {
    case MoveSize::Tiny:
        return 0.5f;

    case MoveSize::Small:
        return 2.f;

    case MoveSize::Half:
        return 5.f;

    case MoveSize::Normal:
        return 10.f;

    case MoveSize::Big:
        return 25.f;

    default:
        return 2.f;
    };

    return 2.f;
};

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

class $modify(MyEditorUI, EditorUI) {
    struct Fields {
        MoveSize moveSize = MoveSize::Small;

        CCMenu* m_buttonMenu;

        CCMenuItemSpriteExtra* m_moveUpBtn;
        CCMenuItemSpriteExtra* m_moveDownBtn;
        CCMenuItemSpriteExtra* m_moveLeftBtn;
        CCMenuItemSpriteExtra* m_moveRightBtn;
    };

    bool init(LevelEditorLayer * p0) {
        if (!EditorUI::init(p0)) return false;

        // create menu for the buttons
        m_fields->m_buttonMenu = CCMenu::create();
        m_fields->m_buttonMenu->setID("quick-move-menu"_spr);
        m_fields->m_buttonMenu->setPosition({ 315.0f, 125.0f });
        m_fields->m_buttonMenu->setAnchorPoint({ 0.5f, 0.5f });
        m_fields->m_buttonMenu->setContentSize({ 75.0f, 75.0f });
        m_fields->m_buttonMenu->ignoreAnchorPointForPosition(false);

        // create visible background for the menu
        auto buttonMenuBg = CCScale9Sprite::create("GJ_square02.png");
        buttonMenuBg->setContentSize(m_fields->m_buttonMenu->getContentSize());
        buttonMenuBg->ignoreAnchorPointForPosition(false);
        buttonMenuBg->setAnchorPoint({ 0.5, 0.5 });
        buttonMenuBg->setPosition({ m_fields->m_buttonMenu->getContentWidth() / 2.f, m_fields->m_buttonMenu->getContentHeight() / 2.f });

        m_fields->m_buttonMenu->addChild(buttonMenuBg);

        // sprite for all buttons
        auto moveBtnSprite = CCSprite::createWithSpriteFrameName("GJ_button_01.png");
        moveBtnSprite->setScale(0.875f);

        // create the up btn
        m_fields->m_moveUpBtn = CCMenuItemSpriteExtra::create(
            moveBtnSprite,
            this,
            menu_selector(MyEditorUI::onMoveUpButton)
        );
        m_fields->m_moveUpBtn->setID("move-up");
        m_fields->m_moveUpBtn->ignoreAnchorPointForPosition(false);
        m_fields->m_moveUpBtn->setAnchorPoint({ 0.5, 0.5 });
        m_fields->m_moveUpBtn->setPosition({ m_fields->m_buttonMenu->getContentWidth() / 2.f, (m_fields->m_buttonMenu->getContentHeight() / 2.f) + 10.f });

        // btn icon sprite
        auto moveBtnIconSpriteName = "edit_upBtn_001.png";

        // move up button
        auto moveUpBtnIcon = CCSprite::createWithSpriteFrameName(moveBtnIconSpriteName);
        moveUpBtnIcon->setScale(0.875f);
        moveUpBtnIcon->setAnchorPoint({ 0.5, 0.5 });
        moveUpBtnIcon->ignoreAnchorPointForPosition(false);
        moveUpBtnIcon->setPosition({ m_fields->m_moveUpBtn->getContentWidth() / 2.f, m_fields->m_moveUpBtn->getContentHeight() / 2.f });

        m_fields->m_moveUpBtn->addChild(moveUpBtnIcon);
        m_fields->m_buttonMenu->addChild(m_fields->m_moveUpBtn);

        // create the down btn
        m_fields->m_moveDownBtn = CCMenuItemSpriteExtra::create(
            moveBtnSprite,
            this,
            menu_selector(MyEditorUI::onMoveDownButton)
        );
        m_fields->m_moveDownBtn->setID("move-down");
        m_fields->m_moveDownBtn->ignoreAnchorPointForPosition(false);
        m_fields->m_moveDownBtn->setAnchorPoint({ 0.5, 0.5 });
        m_fields->m_moveDownBtn->setPosition({ m_fields->m_buttonMenu->getContentWidth() / 2.f, (m_fields->m_buttonMenu->getContentHeight() / 2.f) - 10.f });

        // move down button icon
        auto moveDownBtnIcon = CCSprite::createWithSpriteFrameName(moveBtnIconSpriteName);
        moveDownBtnIcon->setScale(0.875f);
        moveDownBtnIcon->setAnchorPoint({ 0.5, 0.5 });
        moveDownBtnIcon->ignoreAnchorPointForPosition(false);
        moveDownBtnIcon->setPosition({ m_fields->m_moveDownBtn->getContentWidth() / 2.f, m_fields->m_moveDownBtn->getContentHeight() / 2.f });

        m_fields->m_moveDownBtn->addChild(moveDownBtnIcon);
        m_fields->m_buttonMenu->addChild(m_fields->m_moveDownBtn);

        // add the whole menu
        this->addChild(m_fields->m_buttonMenu);

        return true;
    };

    void showUI(bool show) {
        EditorUI::showUI(show);

        if (m_fields->m_buttonMenu) {
            m_fields->m_buttonMenu->setVisible(show);
        };
    };

    // Move all objects based on the button being pressed
    void onMove(EditCommand editCommand = EditCommand::SmallUp, float MOVE_OFFSET = 2.f) {
        if (!m_editorLayer) {
            log::error("No editor layer found");
            return;
        };

        // Check for selected objects
        CCArray* objectsToMove = nullptr;

        if (m_selectedObjects && m_selectedObjects->count() > 0) {
            objectsToMove = m_selectedObjects;
        } else if (m_selectedObject) {
            // Handle single object selection
            objectsToMove = CCArray::create();
            objectsToMove->addObject(m_selectedObject);
            objectsToMove->retain();
        } else {
            log::error("No objects selected to move up");
            return;
        };

        log::info("Objects to move count: {}", objectsToMove->count());

        // Store original positions for potential undo (using editor's built-in system)
        CCArray* validObjects = CCArray::create();
        validObjects->retain();

        CCObject* obj;
        CCARRAY_FOREACH(objectsToMove, obj) {
            if (!obj) {
                log::warn("Null object in selected objects array");
                continue;
            };

            auto gameObj = dynamic_cast<GameObject*>(obj);
            if (gameObj) {
                validObjects->addObject(gameObj);
                log::info("Valid object found at ({}, {})",
                          gameObj->getPosition().x, gameObj->getPosition().y);
            } else {
                log::warn("Object is not a GameObject");
            };
        };

        if (validObjects->count() == 0) {
            log::error("No valid GameObjects found to move");
            validObjects->release();

            if (objectsToMove != m_selectedObjects) {
                objectsToMove->release();
            };

            return;
        };

        // Apply the movement with proper undo support using editor's transform system
        if (m_editorLayer) {
            // First, clear selection and select only our objects
            this->deselectAll();

            for (int i = 0; i < validObjects->count(); i++) {
                auto gameObj = static_cast<GameObject*>(validObjects->objectAtIndex(i));
                this->selectObject(gameObj, true);
            };

            // Use the editor's transformObject function which properly handles undo
            for (int i = 0; i < validObjects->count(); i++) {
                auto gameObj = static_cast<GameObject*>(validObjects->objectAtIndex(i));
                CCPoint oldPos = gameObj->getPosition();

                // Use transformObject with move command (1) and create undo (true)
                this->transformObject(gameObj, editCommand, true);

                // Apply the actual movement after transform setup
                CCPoint newPos = CCPoint(oldPos.x, oldPos.y + MOVE_OFFSET);
                gameObj->setPosition(newPos);

                log::info("Moved object from ({}, {}) to ({}, {})",
                          oldPos.x, oldPos.y, newPos.x, newPos.y);
            };

            log::info("Successfully moved {} objects with undo support", validObjects->count());
        };

        // Clean up
        validObjects->release();
        if (objectsToMove != m_selectedObjects) {
            objectsToMove->release();
        };

        // mmhmm love logs
    };

    void onMoveUpButton(CCObject*) {
        this->onMove(getEditCmd(m_fields->moveSize, MoveDirection::Up), getMoveOffset(m_fields->moveSize));
    };

    void onMoveDownButton(CCObject*) {
        this->onMove(getEditCmd(m_fields->moveSize, MoveDirection::Down), getMoveOffset(m_fields->moveSize));
    };
};