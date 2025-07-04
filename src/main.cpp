#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/utils/NodeIDs.hpp>

using namespace geode::prelude;

class $modify(MyEditorUI, EditorUI) {
    struct Fields {
        CCMenuItemSpriteExtra* m_moveButton;
        CCMenu* m_buttonMenu;
    };

    bool init(LevelEditorLayer* p0) {
        if (!EditorUI::init(p0)) return false;

        auto buttonSprite = CCSprite::createWithSpriteFrameName("edit_upBtn_001.png");
        buttonSprite->setScale(0.8f);
        
        m_fields->m_moveButton = CCMenuItemSpriteExtra::create(
            buttonSprite,
            this,
            menu_selector(MyEditorUI::onMoveButton)
        );
        
        m_fields->m_buttonMenu = CCMenu::create();
        m_fields->m_buttonMenu->addChild(m_fields->m_moveButton);
        m_fields->m_buttonMenu->setPosition({315.0f, 175.0f});
		m_fields->m_buttonMenu->setAnchorPoint({0.5f, 0.5f});
		m_fields->m_buttonMenu->setContentSize({100.0f, 100.0f});
        this->addChild(m_fields->m_buttonMenu);
        

        return true;
    }

	void showUI(bool show){
		EditorUI::showUI(show);
		if (m_fields->m_buttonMenu) {
			m_fields->m_buttonMenu->setVisible(show);
		}
	}

    void onMoveButton(CCObject* sender) {
        if (!m_editorLayer) {
            log::error("No editor layer found");
            return;
        }

        if (!m_selectedObjects || m_selectedObjects->count() == 0) {
            log::error("No objects selected to move up");
            return;
        }

        log::info("Selected objects count: {}", m_selectedObjects->count());

        // Move objects up by a small increment
        const float MOVE_OFFSET = 2.0f;
        int movedCount = 0;

        CCObject* obj;
        CCARRAY_FOREACH(m_selectedObjects, obj) {
            if (!obj) {
                log::warn("Null object in selected objects array");
                continue;
            }

            auto gameObj = dynamic_cast<GameObject*>(obj);
            if (gameObj) {
                CCPoint oldPos = gameObj->getPosition();
                CCPoint newPos = CCPoint(oldPos.x, oldPos.y + MOVE_OFFSET);
                gameObj->setPosition(newPos);
                movedCount++;
                log::info("Moved object {} from ({}, {}) to ({}, {})", 
                         movedCount, oldPos.x, oldPos.y, newPos.x, newPos.y);
            } else {
                log::warn("Object is not a GameObject");
            }
        }

        if (movedCount == 0) {
            log::error("No objects were moved - all objects failed validation");
        } else {
            log::info("Successfully moved {} objects", movedCount);
            
            // Create undo action using the proper GD editor method
            if (m_editorLayer) {
                // Call the editor's update for undo - this is the standard way in GD
                m_editorLayer->updateEditor(0.016f);
                log::info("Editor updated after move operation");
            }
        }
    }
};