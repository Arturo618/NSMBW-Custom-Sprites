#include <common.h>
#include <game.h>
#include <g3dhax.h>
#include <sfx.h>

extern "C" float pow(float num, float power);
extern "C" void incCoin(int playerID); // 0x80060250
extern "C" void addScore(int score, int playerID); // 0x80060690
extern "C" bool SpawnEffect(const char*, int, Vec*, S16Vec*, Vec*);

const char* TenCoinNameList [] = {
	"obj_coin",
	NULL
};

class da10Coin : public dEn_c {
	int onCreate();
	int onExecute();
	int onDelete();
	int onDraw();

	mHeapAllocator_c allocator;
	m3d::mdl_c bodyModel;

	s32 timer;
	u32 delay;
	u32 effect;
	u8 type;
	int coinsToGive;
	//char mdl10[];
	int rotSpeed;
	bool collected;
    //void moveIfCollected();
    //int oldPos;
    bool stupidBool;

	static da10Coin *build();

	void updateModelMatrices();
	void playerCollision(ActivePhysics *apThis, ActivePhysics *apOther);
};

/// Mandatory sprite code

da10Coin *da10Coin::build() {
	void *buffer = AllocFromGameHeap1(sizeof(da10Coin));
	return new(buffer) da10Coin;
}

int da10Coin::onCreate() {
    allocator.link(-1, GameHeaps[0], 0, 0x20);

    nw4r::g3d::ResFile rf(getResource("obj_coin", "g3d/obj_coin.brres")); // TODO: obtain a functional model. I am not a modeler.
    bodyModel.setup(rf.GetResMdl("obj_coin"), &allocator, 0x224, 1, 0); // Green coin, to differentiate the model from a general coin.
    SetupTextures_MapObj(&bodyModel, 0);

    allocator.unlink();

	ActivePhysics::Info HitMeBaby;
	HitMeBaby.xDistToCenter = 0.0;
	HitMeBaby.yDistToCenter = -3.0;
	HitMeBaby.xDistToEdge = 12.0;
	HitMeBaby.yDistToEdge = 15.0;
	HitMeBaby.category1 = 0x5;
	HitMeBaby.category2 = 0x0;
	HitMeBaby.bitfield1 = 0x4F;
	HitMeBaby.bitfield2 = 0xFFFFFFFF;
	HitMeBaby.unkShort1C = 0;
	HitMeBaby.callback = &dEn_c::collisionCallback;

	this->aPhysics.initWithStruct(this, &HitMeBaby);
	this->aPhysics.addToList();

    // make the coin 2x it's usual size. This is a placeholder.
	this->scale.x = 2.0;
	this->scale.y = 2.0;
	this->scale.z = 2.0;
	this->rotSpeed = 0x200;
	// Movement when it's done with existing
	this->collected = false;
    //this->speed.y = 5.0;
    //this->oldPos = pos.y;
    this->coinsToGive = 10; // Hard-coded now.
    this->stupidBool = false;

    // Centering, I think.
	//this->pos.x -= 120.0;
	//this->pos.z = 3300.0;

	this->onExecute();
	return true;

}

int da10Coin::onDelete() {
return true;
}

int da10Coin::onDraw() {
    bodyModel.scheduleForDrawing();
	return true;
}

void da10Coin::updateModelMatrices() {
	matrix.translation(pos.x, pos.y, pos.z);
	matrix.applyRotationYXZ(&rot.x, &rot.y, &rot.z);

	bodyModel.setDrawMatrix(matrix);
	bodyModel.setScale(&scale);
	bodyModel.calcWorld(false);
}

int da10Coin::onExecute() {
	updateModelMatrices();
	this->rot.y += this->rotSpeed; // Notes: X is the vertical axis here. Y is the horizontal. Z... is an axis. General coin movement is in the X axis.
	return true;
}

/// Player collision.

void da10Coin::playerCollision(ActivePhysics *apThis, ActivePhysics *apOther) {
    if (!stupidBool) {
        stupidBool = true;
        this->collected = true; // Let it move!
        for (int i=0; i < this->coinsToGive; i++) {
            incCoin(1);
        }
        SpawnEffect("Wm_ob_starcoinget", 0, &this->pos, &(S16Vec){0,0,0}, &(Vec){1.0, 1.0, 1.0});
        PlaySound(this, SE_OBJ_GET_DRAGON_COIN); // Why would you name it that?
        addScore(1000, 1);
        this->Delete(this->deleteForever);
    }
}
