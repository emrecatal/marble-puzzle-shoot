#include <raylib.h> 
#include <stdlib.h>
#include <math.h>
#define MAX_BALL 10
const int screenWidth = 1500;
const int screenHeight = 800;

typedef enum { GAMEPLAY, LOSING, WINNING } GameScreen;

typedef struct target {
	float x;
	float y;
	float radius;
	Color color;
	bool moving;
}target;

typedef struct node {
	target* data;
	struct node* next;
	struct node* previous;
}node;

typedef struct {
	Vector2 ballPos;
	Vector2 ballSpeed;
	float radius;
	Color color;
	bool isFired;
	bool active;
}bullet;


node* head = NULL;
target hedef[MAX_BALL] = { 0 };
bullet mermi = { 0 };
Vector2 mouse = { 0 };
double aimingAngle = 0;
Rectangle health = { 20,20,120,15 };
int healthCounter = 0;
int num = 0;

void initGame();
void updateGame();
void targetCreator(node**, target*);
void drawTargets(node*);
void freeTargets(node*);
void updateTarget(node*);
void bulletFire();
Color giveColor();
Color giveColorBullet(node*);
bool isSameColor(Color, Color);
GameScreen updateScreen(int);
int isWinning(node*);

int checkCollision(node**, bullet*);
target* shotTargetIndex(node*, bullet*);
int whereTarget(node*);
target* createOne(bullet);
node* addTargetBetween(target* newCreated, target* shotTargetIndex);
void stepBack(node*, node*);
void isBoom(node* addedNode);

int main(void) {
	InitWindow(screenWidth, screenHeight, "marble puzzle shoot");
	SetTargetFPS(120);
	initGame();

	while (!WindowShouldClose()) {
		ClearBackground(LIGHTGRAY);

		updateGame();
		if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) bulletFire();

		BeginDrawing();
		drawTargets(head);
		DrawCircle(screenWidth / 2, screenHeight / 2, 40.0, DARKGREEN);
		DrawCircle(550, screenHeight / 2, 30, DARKGREEN);
		DrawRectangleRec(health, RED);
		DrawText("HEALTH", 45, 20, 18, LIGHTGRAY);
		if (mermi.active == true) DrawCircle(mermi.ballPos.x, mermi.ballPos.y, 20, mermi.color);
		EndDrawing();
	}
	freeTargets(head);
	CloseWindow();
	return 0;
}

void initGame() {
	for (num = 0; num < MAX_BALL; num++) {
		hedef[num].x = 80;
		hedef[num].y = 80 - num * 40;
		hedef[num].radius = 20;
		hedef[num].color = giveColor();
		hedef[num].moving = true;

		targetCreator(&head, &hedef[num]);
	}

	mermi.ballPos = (Vector2){ screenWidth / 2, screenHeight / 2 };
	mermi.ballSpeed = (Vector2){ 0 , 0 };
	mermi.radius = 20.0;
	mermi.color = giveColorBullet(head);
	mermi.isFired = false;
	mermi.active = true;
}

target* createOne(bullet mermi) {
	target* newCreated = (target*)malloc(sizeof(target));
	if (newCreated != NULL) {
		node* current = head;

		while (current->data != shotTargetIndex(head, &mermi)) {
			current = current->next;
		}

		switch (whereTarget(current)) {
		case 1: // aþaðý
			newCreated->x = current->data->x;
			newCreated->y = current->data->y - 40;
			break;
		case 2: // yukarý
			newCreated->x = current->data->x;
			newCreated->y = current->data->y + 40;
			break;
		case 3: //saða
			newCreated->x = current->data->x - 40;
			newCreated->y = current->data->y;
			break;
		case 4: // sola
			newCreated->x = current->data->x + 40;
			newCreated->y = current->data->y;
			break;
		}

		newCreated->radius = 20;
		newCreated->color = mermi.color;
		newCreated->moving = true;
		num++;
	}
	return newCreated;
	addTargetBetween(newCreated, shotTargetIndex(head, &mermi));
}

/*node* addTargetBetween(target* newCreated, target* shotTargetIndex) {
	node* current = head;
	node* shotted = NULL;

	while (current->next != NULL) {
		current = current->next;
	}

	node* new = (node*)malloc(sizeof(node));
	if (new == NULL) return NULL;
	
	if (current->data == shotTargetIndex) {	
		shotted = current->previous; 

		new->data = newCreated;

		new->next = current;
		new->previous = shotted;
		current->previous = new;
		shotted->next = new;
		return new;
	}

	while (current->previous->data != shotTargetIndex) {
		current = current->previous;
	}
		shotted = current->previous; // shotted should be the node before current

		new->data = newCreated;

		new->next = current;
		new->previous = shotted;
		current->previous = new;
		if (shotted != NULL) {
			shotted->next = new;
		}
		else {
			head = new;
		}
	
	return new;
} */



node* addTargetBetween(target* newCreated, target* shotTargetIndex) {
	node* current = head;
	node* shotted = NULL;

	while (current->next != NULL) {
		current = current->next;
	}

	node* new = (node*)malloc(sizeof(node));
	if (new == NULL) return;


	if (current->data == shotTargetIndex) {
		shotted = current->previous; //shotted deðil önceki
		new->data = newCreated;

		new->next = current;
		new->previous = shotted;
		current->previous = new;
		shotted->next = new;
		return new;
	}

	while (current->previous->data != shotTargetIndex) {
		current = current->previous;
	}

	shotted = current->previous;

	new->data = newCreated;

	new->next = current;
	new->previous = shotted;
	current->previous = new;
	shotted->next = new;

	return new;
} 

void updateGame() {
	updateTarget(head);


	mermi.ballPos.x += mermi.ballSpeed.x;
	mermi.ballPos.y += mermi.ballSpeed.y;

	if (mermi.isFired == true) {
		mermi.ballPos.x += cos(aimingAngle) * 4.0f;
		mermi.ballPos.y += sin(aimingAngle) * 4.0f;
	}

	if (mermi.ballPos.x > (float)screenWidth + 20.0 || mermi.ballPos.x < -20.0 || mermi.ballPos.y >(float)screenHeight + 20.0 || mermi.ballPos.y < -20.0) {
		mermi.active = false;
	}
	if (mermi.active == false) {
		mermi.color = giveColorBullet(head);
		mermi.ballPos = (Vector2){ screenWidth / 2, screenHeight / 2 };
		mermi.ballSpeed.x = 0.0;
		mermi.ballSpeed.y = 0.0;
		mermi.isFired = false;
		mermi.active = true;
	}

	if (checkCollision(head, &mermi)) {
		createOne(mermi);
		stepBack(head, addTargetBetween(createOne(mermi),shotTargetIndex(head, &mermi)));
		isBoom(addTargetBetween(createOne(mermi), shotTargetIndex(head, &mermi)));
	}

}

void updateTarget(node* head) {
	node* current = head;

	while (current != NULL) {
		target* selected = current->data;
		if (selected->moving == true) {

			if ((selected->x == 80) && (selected->y < screenHeight - 80))selected->y++;
			if ((selected->y == screenHeight - 80) && (selected->x < screenWidth - 80)) selected->x++;
			if ((selected->x == screenWidth - 80) && (selected->y > 80))selected->y--;
			if ((selected->y == 80) && (selected->x > 300)) current->data->x--;
			if ((selected->x == 300) && (selected->y < screenHeight - 160)) current->data->y++;

			if ((selected->y == screenHeight - 160) && (selected->x < screenWidth - 300) && (selected->x != 80)) selected->x++;
			if ((selected->x == screenWidth - 300) && (selected->y > 160) && (selected->y != screenHeight - 80)) selected->y--;
			if ((selected->y == 160) && (selected->x > 550) && (selected->x != screenWidth - 80)) selected->x--;
			if ((selected->x == 550) && (selected->y < screenHeight / 2) && (selected->y != 80)) selected->y++;


			if ((selected->x == 550) && (selected->y == screenHeight / 2 - 10) && (selected->moving == true)) healthCounter++;
			if ((health.width >= 80) && (healthCounter == 1)) health.width--;
			if ((health.width >= 40) && (healthCounter == 2)) health.width--;
			if ((health.width >= 0) && (healthCounter == 3)) health.width--;
		}
		current = current->next;
	}
}

int whereTarget(node* given) {
	node* current = given;

	target* selected = current->data;
	if ((selected->x == 80) && (selected->y < screenHeight - 80)) return 1; //aþaðý gidiyor
	if ((selected->y == screenHeight - 80) && (selected->x < screenWidth - 80)) return 3; // saða gidiyor
	if ((selected->x == screenWidth - 80) && (selected->y > 80)) return 2; // yukarý gidiyor
	if ((selected->y == 80) && (selected->x > 300)) return 4; // sola gidiyor
	if ((selected->x == 300) && (selected->y < screenHeight - 160)) return 1;

	if ((selected->y == screenHeight - 160) && (selected->x < screenWidth - 300) && (selected->x != 80)) return 3;
	if ((selected->x == screenWidth - 300) && (selected->y > 160) && (selected->y != screenHeight - 80)) return 2;
	if ((selected->y == 160) && (selected->x > 550) && (selected->x != screenWidth - 80)) return 4;
	if ((selected->x == 550) && (selected->y < screenHeight / 2) && (selected->y != 80)) return 1;
}

void targetCreator(node** head, target* hedef) {
	node* new_node = (node*)malloc(sizeof(node));
	if (new_node == NULL) {
		return;
	}

	new_node->data = (target*)malloc(sizeof(target));
	if (new_node->data == NULL) {
		free(new_node);
		return;
	}

	new_node->data->x = hedef->x;
	new_node->data->y = hedef->y;
	new_node->data->radius = hedef->radius;
	new_node->data->color = hedef->color;
	new_node->data->moving = hedef->moving;
	new_node->next = NULL;
	new_node->previous = NULL;

	if (*head == NULL) {
		*head = new_node;
	}
	else {
		node* current = *head;
		while (current->next != NULL) {
			current = current->next;
		}
		current->next = new_node;
		new_node->previous = current;
	}
}

void drawTargets(node* head) {
	node* current = head;
	while (current != NULL) {
		DrawCircle(current->data->x, current->data->y, current->data->radius, current->data->color);
		current = current->next;
	}
}

void freeTargets(node* head) {
	node* current = head;
	while (current != NULL) {
		node* next = current->next;
		free(current->data);
		free(current);
		current = next;
	}
}

int checkCollision(node* head, bullet* mermi) {
	node* current = head;

	while (current != NULL) {
		Vector2 hedefCenter = { current->data->x, current->data->y };
		Vector2 mermiCenter = { mermi->ballPos.x, mermi->ballPos.y };
		if (CheckCollisionCircles(hedefCenter, 20, mermiCenter, 20)) {
			mermi->active = false;
			mermi->isFired = false;
			return 1;
		}
		current = current->next;
	}
	return 0;
}

target* shotTargetIndex(node* head, bullet* mermi) {
	node* current = head;

	while (current != NULL) {
		Vector2 hedefCenter = { current->data->x, current->data->y };
		Vector2 mermiCenter = { mermi->ballPos.x, mermi->ballPos.y };

		if (CheckCollisionCircles(hedefCenter, 20, mermiCenter, 20)) {
			return current->data;
		}
		current = current->next;
	}
}

void stepBack(node* head, node* newCreated) {
	node* current = head;

	while (current->next != NULL) {
		current = current->next;
	}

	while (current->previous != NULL && current->previous != newCreated) {
		current = current->previous;
	}

	current = current->previous;


	while (current != NULL) {
		int hamle = 40;

		switch (whereTarget(current)) {
		case 1: // aþaðý
			if (current->data->x == 80) {

				while (current->data->y < screenHeight - 80 && hamle > 0) {
					current->data->y += 1;
					hamle -= 1;
				}
				while (current->data->y == screenHeight - 80 && hamle > 0) {
					current->data->x += 1;
					hamle -= 1;
				}

			}
			else if (current->data->x == 300) {

				while (current->data->y < screenHeight - 160 && hamle > 0) {
					current->data->y += 1;
					hamle -= 1;
				}
				while (current->data->y == screenHeight - 160 && hamle > 0) {
					current->data->x += 1;
					hamle -= 1;
				}

			}
			else if (current->data->x == 550) {

				while (current->data->y < screenHeight / 2 && hamle > 0) {
					current->data->y += 1;
					hamle -= 1;
				}
				while (current->data->y >= screenHeight / 2 && hamle > 0) {
					current->data->y = screenHeight / 2;
					hamle -= 1;
				}

			}
			break;

		case 2: // yukarý
			if (current->data->x == screenWidth - 80) {

				while (current->data->y > 80 && hamle > 0) {
					current->data->y -= 1;
					hamle -= 1;
				}
				while (current->data->y == 80 && hamle > 0) {
					current->data->x -= 1;
					hamle -= 1;
				}

			}
			else if (current->data->x == screenWidth - 300) {

				while (current->data->y > 160 && hamle > 0) {
					current->data->y -= 1;
					hamle -= 1;
				}
				while (current->data->y == 160 && hamle > 0) {
					current->data->x -= 1;
					hamle -= 1;
				}

			}
			break;

		case 3: //saða
			if (current->data->y == screenHeight - 80) {

				while (current->data->x < screenWidth - 80 && hamle > 0) {
					current->data->x += 1;
					hamle -= 1;
				}
				while (current->data->x == screenWidth - 80 && hamle > 0) {
					current->data->y -= 1;
					hamle -= 1;
				}

			}
			else if (current->data->y == screenHeight - 160) {

				while (current->data->x < screenWidth - 300 && hamle > 0) {
					current->data->x += 1;
					hamle -= 1;
				}
				while (current->data->x == screenWidth - 300 && hamle > 0) {
					current->data->y -= 1;
					hamle -= 1;
				}

			}

		case 4: // sola
			if (current->data->y == 80) {

				while (current->data->x > 300 && hamle > 0) {
					current->data->x -= 1;
					hamle -= 1;
				}
				while (current->data->x == 300 && hamle > 0) {
					current->data->y += 1;
					hamle -= 1;
				}

			}
			else if (current->data->y == 160) {

				while (current->data->x > 550 && hamle > 0) {
					current->data->x -= 1;
					hamle -= 1;
				}
				while (current->data->x == 550 && hamle > 0) {
					current->data->y += 1;
					hamle -= 1;
				}

			}
			break;
		}
		current = current->previous;
	}
}

void isBoom(node* addTargetBetween){
	node* newNode = addTargetBetween;
	
	if (isSameColor(newNode->data->color, newNode->next->data->color) && isSameColor(newNode->data->color, newNode->previous->data->color)) {
		newNode->data->moving = false;
		newNode->previous->next = newNode->next;
		newNode->next->previous = newNode->previous;
		free(newNode);
	}
}

void bulletFire() {
	if (mermi.isFired == false && mermi.active == true) {
		mermi.ballPos = (Vector2){ screenWidth / 2, screenHeight / 2 };
		Vector2 mouse = GetMousePosition();

		if (mouse.x > screenWidth / 2 && mouse.y < screenHeight / 2) aimingAngle = atan(-(mermi.ballPos.y - mouse.y) / (mouse.x - mermi.ballPos.x));
		if (mouse.x < screenWidth / 2 && mouse.y < screenHeight / 2) aimingAngle = PI - atan((mermi.ballPos.y - mouse.y) / (mouse.x - mermi.ballPos.x));
		if (mouse.x < screenWidth / 2 && mouse.y > screenHeight / 2) aimingAngle = (PI)-atan((mermi.ballPos.y - mouse.y) / (mouse.x - mermi.ballPos.x));
		if (mouse.x > screenWidth / 2 && mouse.y > screenHeight / 2) aimingAngle = (2 * PI) - atan((mermi.ballPos.y - mouse.y) / (mouse.x - mermi.ballPos.x));

		mermi.isFired = true;
	}
}

Color giveColor() {
	int random = GetRandomValue(1, 3);
	if (random == 1) return RED;
	else if (random == 2) return BLUE;
	else return GREEN;
}

Color giveColorBullet(node* head) {
	node* current = head;
	int random = GetRandomValue(1, 3);

	while (current != NULL) {
		if (random == 1 && isSameColor(current->data->color, RED)) {
			return RED;
		}
		if (random == 2 && isSameColor(current->data->color, BLUE)) {
			return BLUE;
		}
		if (random == 3 && isSameColor(current->data->color, GREEN)) {
			return GREEN;
		}
		current = current->next;
	}
	giveColorBullet(head);
}

bool isSameColor(Color color1, Color color2) { 
	return (color1.r == color2.r && color1.g == color2.g && color1.b == color2.b && color1.a == color2.a);
}

GameScreen updateScreen(int healthcounter) {
	switch (healthCounter) {
	case 0: case 1: case 2:
		return GAMEPLAY;
		if (isWinning(head)) return WINNING; break;

	default:
		return LOSING; break;
	}
}

int isWinning(node* head) {
	node* current = head;
	int passiveCounter = 0;

	while (current != NULL) {
		if (current->data->x == 550 && current->data->y == screenHeight / 2) current->data->moving = false;
		if (current->data->moving == false) passiveCounter++;
		current = current->next;
	}
	if (passiveCounter >= MAX_BALL) return 1;
	return 0;
}