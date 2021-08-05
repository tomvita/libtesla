#define TESLA_INIT_IMPL // If you have more than one file using the tesla header, only define this in the main one
#include <tesla.hpp>    // The Tesla Header
#include "dmntcht.h"
#include "debugger.hpp"
#include "memory_dump.hpp"
// Bookmark display
#define MAX_POINTER_DEPTH 12
  struct pointer_chain_t
  {
    u64 depth = 0;
    s64 offset[MAX_POINTER_DEPTH + 1] = {0}; // offset to address pointed by pointer
  };
struct bookmark_t {
    char label[19] = {0};
    searchType_t type;
    pointer_chain_t pointer;
    bool heap = true;
    u64 offset = 0;
    bool deleted = false;
};
#define NUM_bookmark 10
char BookmarkLabels[NUM_bookmark * 20] = "";
char Variables[NUM_bookmark*20];
Result dmntchtCheck = 1;
std::string m_edizon_dir = ""; 
std::string m_store_extension = "";
Debugger *m_debugger; 
MemoryDump *m_memoryDump;
MemoryDump *m_AttributeDumpBookmark;
u8 m_addresslist_offset = 0;
bool m_32bitmode = false;
// static const std::vector<u8> dataTypeSizes = {1, 1, 2, 2, 4, 4, 8, 8, 4, 8, 8};
DmntCheatProcessMetadata metadata;
char bookmarkfilename[200]="bookmark filename";
void init_se_tools() {
    dmntchtCheck = dmntchtInitialize();
    dmntchtGetCheatProcessMetadata(&metadata);
	u8 build_id[0x20];
    memcpy(build_id, metadata.main_nso_build_id, 0x20);

    m_debugger = new Debugger();

	// check and set m_32bitmode

	snprintf(bookmarkfilename, 200, "%s/%02X%02X%02X%02X%02X%02X%02X%02X.dat", EDIZON_DIR,
                     build_id[0], build_id[1], build_id[2], build_id[3], build_id[4], build_id[5], build_id[6], build_id[7]);
	return;

    m_AttributeDumpBookmark = new MemoryDump(bookmarkfilename, DumpType::ADDR, false);
	m_memoryDump = new MemoryDump(EDIZON_DIR "/memdumpbookmark.dat", DumpType::ADDR, false);

};
void cleanup_se_tools() {
    if (dmntchtCheck == 0) dmntchtExit();
    delete m_debugger;
	return;
};
static std::string _getAddressDisplayString(u64 address, Debugger *debugger, searchType_t searchType) {
    char ss[200];
    searchValue_t searchValue;
    searchValue._u64 = debugger->peekMemory(address);
    {
        switch (searchType) {
            case SEARCH_TYPE_UNSIGNED_8BIT:
                snprintf(ss, sizeof ss, "%d", searchValue._u8);
                break;
            case SEARCH_TYPE_UNSIGNED_16BIT:
                snprintf(ss, sizeof ss, "%d", searchValue._u16);
                break;
            case SEARCH_TYPE_UNSIGNED_32BIT:
                snprintf(ss, sizeof ss, "%d", searchValue._u32);
                break;
            case SEARCH_TYPE_UNSIGNED_64BIT:
                snprintf(ss, sizeof ss, "%ld", searchValue._u64);
                break;
            case SEARCH_TYPE_SIGNED_8BIT:
                snprintf(ss, sizeof ss, "%d", searchValue._s8);
                break;
            case SEARCH_TYPE_SIGNED_16BIT:
                snprintf(ss, sizeof ss, "%d", searchValue._s16);
                break;
            case SEARCH_TYPE_SIGNED_32BIT:
                snprintf(ss, sizeof ss, "%d", searchValue._s32);
                break;
            case SEARCH_TYPE_SIGNED_64BIT:
                snprintf(ss, sizeof ss, "%ld", searchValue._s64);
                break;
            case SEARCH_TYPE_FLOAT_32BIT:
                snprintf(ss, sizeof ss, "%f", searchValue._f32);
                break;
            case SEARCH_TYPE_FLOAT_64BIT:
                snprintf(ss, sizeof ss, "%lf", searchValue._f64);
                break;
            case SEARCH_TYPE_POINTER:
                snprintf(ss, sizeof ss, "0x%016lX", searchValue._u64);
                break;
            case SEARCH_TYPE_NONE:
                break;
        }
    }
    return ss;  //.str();
}

class BookmarkOverlay : public tsl::Gui {
public:
    BookmarkOverlay() { }

    virtual tsl::elm::Element* createUI() override {
		
		auto *rootFrame = new tsl::elm::OverlayFrame("EdiZon SE bookmarks", bookmarkfilename);

		auto Status = new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {
			
			// if (GameRunning == false) renderer->drawRect(0, 0, tsl::cfg::FramebufferWidth - 150, 180, a(0x7111));
			// else 
            renderer->drawRect(0, 0, tsl::cfg::FramebufferWidth - 150, 110, a(0x7111));
			

			renderer->drawString(BookmarkLabels, false, 0, 15, 15, renderer->a(0xFF0F));
			
			renderer->drawString(Variables, false, 60, 15, 15, renderer->a(0xFF0F));
		});

		rootFrame->setContent(Status);

		return rootFrame;
	}

	virtual void update() override {
		// if (TeslaFPS == 60) TeslaFPS = 1;
		
		// snprintf(FPS_var_compressed_c, sizeof FPS_compressed_c, "%u\n%2.2f", FPS, FPSavg);
snprintf(BookmarkLabels,sizeof BookmarkLabels,"label\nlabe\nGame Runing = %d\n%s\nSaltySD = %d\ndmntchtCheck = 0x%08x\n",1,bookmarkfilename,1,dmntchtCheck);
snprintf(Variables,sizeof Variables, "100\n200\n");
// strcat(BookmarkLabels,bookmarkfilename);
return;
		// snprintf(Variables, sizeof Variables, "%d\n%d\n%d\n%s\n%s", Bstate.A, Bstate.B, TeslaFPS, skin_temperature_c, Rotation_SpeedLevel_c);
		for (u8 line = 0; line < NUM_bookmark; line++) {
			if ((line + m_addresslist_offset) >= (m_memoryDump->size() / sizeof(u64)))
				break;

			// std::stringstream ss;
			// ss.str("");
			char ss[200] = "";
			bookmark_t bookmark;
			// if (line < NUM_bookmark)  // && (m_memoryDump->size() / sizeof(u64)) != 8)
			{
				u64 address = 0;
				m_memoryDump->getData((line + m_addresslist_offset) * sizeof(u64), &address, sizeof(u64));
				m_AttributeDumpBookmark->getData((line + m_addresslist_offset) * sizeof(bookmark_t), &bookmark, sizeof(bookmark_t));
				// if (false)
				if (bookmark.pointer.depth > 0)  // check if pointer chain point to valid address update address if necessary
				{
					bool updateaddress = true;
					u64 nextaddress = metadata.main_nso_extents.base; //m_mainBaseAddr;
					for (int z = bookmark.pointer.depth; z >= 0; z--) {
						nextaddress += bookmark.pointer.offset[z];
						MemoryInfo meminfo = m_debugger->queryMemory(nextaddress);
						if (meminfo.perm == Perm_Rw)
							if (z == 0) {
								if (address == nextaddress)
									updateaddress = false;
								else {
									address = nextaddress;
								}
							} else
								m_debugger->readMemory(&nextaddress, ((m_32bitmode) ? sizeof(u32) : sizeof(u64)), nextaddress);
						else {
							updateaddress = false;
							break;
						}
					}
					if (updateaddress) {
						m_memoryDump->putData((line + m_addresslist_offset) * sizeof(u64), &address, sizeof(u64));
						m_memoryDump->flushBuffer();
					}
				}
				// bookmark display
				snprintf(ss, sizeof ss, "%s\n", _getAddressDisplayString(address, m_debugger, (searchType_t)bookmark.type).c_str());
				strcat(Variables,ss);
				snprintf(ss, sizeof ss, "%s\n", bookmark.label);
				strcat(BookmarkLabels,ss);
			} 
		}
    };
    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
		if ((keysHeld & HidNpadButton_StickL) && (keysHeld & HidNpadButton_StickR)) {
			// CloseThreads();
			// cleanup_se_tools();
			tsl::goBack();
			return true;
		};
		if (keysDown & HidNpadButton_B && keysHeld & HidNpadButton_ZL) {
			// CloseThreads();
			// cleanup_se_tools();
			tsl::goBack();
			return true;
		}
        return false;   // Return true here to signal the inputs have been consumed
    }
};
class GuiSecondary : public tsl::Gui {
public:
    GuiSecondary() {}

    virtual tsl::elm::Element* createUI() override {
        auto *rootFrame = new tsl::elm::OverlayFrame("Tesla Example", "v1.3.2 - Secondary Gui");

        rootFrame->setContent(new tsl::elm::DebugRectangle(tsl::Color{ 0x8, 0x3, 0x8, 0xF }));

        return rootFrame;
    }
};

class GuiTest : public tsl::Gui {
public:
    GuiTest(u8 arg1, u8 arg2, bool arg3) { }

    // Called when this Gui gets loaded to create the UI
    // Allocate all elements on the heap. libtesla will make sure to clean them up when not needed anymore
    virtual tsl::elm::Element* createUI() override {
        // A OverlayFrame is the base element every overlay consists of. This will draw the default Title and Subtitle.
        // If you need more information in the header or want to change it's look, use a HeaderOverlayFrame.
        auto frame = new tsl::elm::OverlayFrame("Tesla Example", "v1.3.2");

        // A list that can contain sub elements and handles scrolling
        auto list = new tsl::elm::List();

        // List Items
        list->addItem(new tsl::elm::CategoryHeader("List items"));

        auto *clickableListItem = new tsl::elm::ListItem("BookmarkOverlay", "...");
        clickableListItem->setClickListener([](u64 keys) {
            if (keys & HidNpadButton_A) {
                tsl::changeTo<BookmarkOverlay>();
                return true;
            }

            return false;
        });

        list->addItem(clickableListItem);
        list->addItem(new tsl::elm::ListItem("Default List Item"));
        list->addItem(new tsl::elm::ListItem("Default List Item with an extra long name to trigger truncation and scrolling"));
        list->addItem(new tsl::elm::ToggleListItem("Toggle List Item", true));

        // Custom Drawer, a element that gives direct access to the renderer
        list->addItem(new tsl::elm::CategoryHeader("Custom Drawer", true));
        list->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            renderer->drawCircle(x + 40, y + 40, 20, true, renderer->a(0xF00F));
            renderer->drawCircle(x + 50, y + 50, 20, true, renderer->a(0xF0F0));
            renderer->drawRect(x + 130, y + 30, 60, 40, renderer->a(0xFF00));
            renderer->drawString("Hello :)", false, x + 250, y + 70, 20, renderer->a(0xFF0F));
            renderer->drawRect(x + 40, y + 90, 300, 10, renderer->a(0xF0FF));
        }), 100);

        // Track bars
        list->addItem(new tsl::elm::CategoryHeader("Track bars"));
        list->addItem(new tsl::elm::TrackBar("\u2600"));
        list->addItem(new tsl::elm::StepTrackBar("\uE13C", 20));
        list->addItem(new tsl::elm::NamedStepTrackBar("\uE132", { "Selection 1", "Selection 2", "Selection 3" }));

        // Add the list to the frame for it to be drawn
        frame->setContent(list);

        // Return the frame to have it become the top level element of this Gui
        return frame;
    }

    // Called once every frame to update values
    virtual void update() override {

    }

    // Called once every frame to handle inputs not handled by other UI elements
    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override {
        return false;   // Return true here to signal the inputs have been consumed
    }
};

class OverlayTest : public tsl::Overlay {
public:
                                             // libtesla already initialized fs, hid, pl, pmdmnt, hid:sys and set:sys
    virtual void initServices() override {
        init_se_tools();
    }  // Called at the start to initialize all services necessary for this Overlay
    virtual void exitServices() override {
        cleanup_se_tools();
    }  // Called at the end to clean up all services previously initialized

    virtual void onShow() override {}    // Called before overlay wants to change from invisible to visible state
    virtual void onHide() override {}    // Called before overlay wants to change from visible to invisible state

    virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return initially<GuiTest>(1, 2, true);  // Initial Gui to load. It's possible to pass arguments to it's constructor like this
    }
};

int main(int argc, char **argv) {
    return tsl::loop<OverlayTest>(argc, argv);
}
