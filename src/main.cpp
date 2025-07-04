#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/utils/NodeIDs.hpp>

using namespace geode::prelude;

class $modify(MyEditorUI, EditorUI) {
    struct Fields {
        CCMenuItemSpriteExtra* m_moveUpBtn;

        CCMenu* m_buttonMenu;
    };

    bool init(LevelEditorLayer * p0) {
        if (!EditorUI::init(p0)) return false;

        // create menu for the buttons
        m_fields->m_buttonMenu = CCMenu::create();
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
        m_fields->m_moveUpBtn->ignoreAnchorPointForPosition(false);
        m_fields->m_moveUpBtn->setAnchorPoint({ 0.5, 0.5 });
        m_fields->m_moveUpBtn->setPosition({ 35.f, 50.f });

        // up btn icon sprite
        auto moveUpBtnIcon = CCSprite::createWithSpriteFrameName("edit_upBtn_001.png");
        moveUpBtnIcon->setScale(0.875f);
        moveUpBtnIcon->setAnchorPoint({ 0.5, 0.5 });
        moveUpBtnIcon->ignoreAnchorPointForPosition(false);
        moveUpBtnIcon->setPosition({ m_fields->m_moveUpBtn->getContentWidth() / 2.f, m_fields->m_moveUpBtn->getContentHeight() / 2.f });

        m_fields->m_moveUpBtn->addChild(moveUpBtnIcon);
        m_fields->m_buttonMenu->addChild(m_fields->m_moveUpBtn);

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

    void onMoveUpButton(CCObject * sender) {
        if (!m_editorLayer) {
            log::error("No editor layer found");
            return;
        }

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

        // Move objects up by a small increment (2 units)
        const float MOVE_OFFSET = 2.0f;

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
            }
            return;
        }

        // Apply the movement with proper undo support using editor's transform system
        if (m_editorLayer) {
            // First, clear selection and select only our objects
            this->deselectAll();

            for (int i = 0; i < validObjects->count(); i++) {
                auto gameObj = static_cast<GameObject*>(validObjects->objectAtIndex(i));
                this->selectObject(gameObj, true);
            }

            // Use the editor's transformObject function which properly handles undo
            for (int i = 0; i < validObjects->count(); i++) {
                auto gameObj = static_cast<GameObject*>(validObjects->objectAtIndex(i));
                CCPoint oldPos = gameObj->getPosition();

                // Use transformObject with move command (1) and create undo (true)
                this->transformObject(gameObj, EditCommand::SmallUp, true);

                // Apply the actual movement after transform setup
                CCPoint newPos = CCPoint(oldPos.x, oldPos.y + MOVE_OFFSET);
                gameObj->setPosition(newPos);

                log::info("Moved object from ({}, {}) to ({}, {})",
                          oldPos.x, oldPos.y, newPos.x, newPos.y);
            }

            log::info("Successfully moved {} objects with undo support", validObjects->count());
        }

        // Clean up
        validObjects->release();
        if (objectsToMove != m_selectedObjects) {
            objectsToMove->release();
        }

        // mmhmm love logs
    }
};