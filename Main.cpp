//Uwaga! Co najmniej C++17!!!
//Project-> ... Properties->Configuration Properties->General->C++ Language Standard = ISO C++ 17 Standard (/std:c++17)

#include <SFML/Graphics.hpp>
#include <fstream>
#include <cmath>
#include <iostream>

enum class Field { VOID, FLOOR, WALL, BOX, PARK, PLAYER };

class Sokoban : public sf::Drawable
{
public:
	void LoadMapFromFile(std::string fileName);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	void SetDrawParameters(sf::Vector2u draw_area_size);
	void Move_Player_Left();
	void Move_Player_Right();
	void Move_Player_Up();
	void Move_Player_Down();
	bool Is_Victory() const;
	std::vector<std::vector<Field>> map;
	sf::Vector2f shift; //przesunięcie
	sf::Vector2f tile_size; //rozmiar kafelka
	sf::Vector2i player_position;
	std::vector<sf::Vector2i> park_positions; //pozycje końcowe

	void move_player(int dx, int dy);
};

void Sokoban::LoadMapFromFile(std::string fileName)
{
	std::string str;
	std::vector<std::string> vos;

	std::ifstream in(fileName.c_str());
	while (std::getline(in, str)) { vos.push_back(str); }
	in.close();

	map.clear();
	map.resize(vos.size(), std::vector<Field>(vos[0].size()));
	for (auto [row, row_end, y] = std::tuple{ vos.cbegin(), vos.cend(), 0 }; row != row_end; ++row, ++y)
		for (auto [element, end, x] = std::tuple{ row->begin(), row->end(), 0 }; element != end; ++element, ++x)
			switch (*element)
			{
			case 'X': map[y][x] = Field::WALL; break;
			case '*': map[y][x] = Field::VOID; break;
			case ' ': map[y][x] = Field::FLOOR; break;
			case 'B': map[y][x] = Field::BOX; break;
			case 'P': map[y][x] = Field::PARK; park_positions.push_back(sf::Vector2i(x,y));  break;
			case 'S': map[y][x] = Field::PLAYER; player_position = sf::Vector2i(x,y);  break;
			}
}

void Sokoban::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	// Tu niewątpliwie powinno coś być : -) Tu należy narysować wszystko. O tak jakoś :
	//target.draw(....);

	//Przydatna może być pętla :
	sf::Texture textura;
	sf::Sprite sprite;
	for (int y = 0; y < map.size(); ++y)
		for (int x = 0; x < map[y].size(); ++x)
		{
			switch (map[y][x])
			{
			/*case Field::WALL: textura.loadFromFile("wall.png"); break;
			case Field::VOID: textura.loadFromFile("void.png"); break;
			case Field::FLOOR: textura.loadFromFile("floor.png"); break;
			case Field::BOX: textura.loadFromFile("box.png"); break;
			case Field::PARK: textura.loadFromFile("park.png"); break;
			case Field::PLAYER: textura.loadFromFile("player.png"); break;*/
			case Field::WALL: textura.loadFromFile("end_bricks.png"); break;
			case Field::VOID: textura.loadFromFile("end_stone.png"); break;
			case Field::FLOOR: textura.loadFromFile("spore_blossom.png"); break;
			case Field::BOX: textura.loadFromFile("ender_eye.png"); break;
			case Field::PARK: textura.loadFromFile("endframe_top.png"); break;
			case Field::PLAYER: textura.loadFromFile("totem.png"); break;
			}
			sprite.setTexture(textura);
   			sprite.setPosition(shift.x+x*tile_size.x,shift.y+y*tile_size.y);
			sprite.setScale(sf::Vector2f(tile_size.x/textura.getSize().x,tile_size.y/textura.getSize().y));
			target.draw(sprite);
		}

}

void Sokoban::SetDrawParameters(sf::Vector2u draw_area_size)
{
	this->tile_size = sf::Vector2f(
		std::min(std::floor((float)draw_area_size.x / (float)map[0].size()), std::floor((float)draw_area_size.y / (float)map.size())), 
		std::min(std::floor((float)draw_area_size.x / (float)map[0].size()), std::floor((float)draw_area_size.y / (float)map.size()))
	);
	this->shift = sf::Vector2f(
		((float)draw_area_size.x - this->tile_size.x * map[0].size()) / 2.0f, 
		((float)draw_area_size.y - this->tile_size.y * map.size()) / 2.0f
	);
}

void Sokoban::Move_Player_Left()
{
	move_player(-1, 0);
}

void Sokoban::Move_Player_Right()
{
	move_player(1, 0);
}

void Sokoban::Move_Player_Up()
{
	move_player(0, -1);
}

void Sokoban::Move_Player_Down()
{
	move_player(0, 1);
}

void Sokoban::move_player(int dx, int dy) 
{
	bool allow_move = false; // Pesymistyczne załóżmy, że gracz nie może się poruszyć.
	sf::Vector2i new_pp(player_position.x + dx,player_position.y + dy); //Potencjalna nowa pozycja gracza.
	Field fts = map[new_pp.y][new_pp.x]; //Element na miejscu na które gracz zamierza przejść.
	Field ftsa = map[new_pp.y + dy][new_pp.x + dx]; //Element na miejscu ZA miejscem na które gracz zamierza przejść. :-D
	
	//Gracz może się poruszyć jeśli pole na którym ma stanąć to podłoga lub miejsce na skrzynki.
	if (fts == Field::FLOOR || fts == Field::PARK) allow_move = true;
	//Jeśli pole na które chce się poruszyć gracz zawiera skrzynkę to może się on poruszyć jedynie jeśli kolejne pole jest puste lub zawiera miejsce na skrzynkę  - bo wtedy może przepchnąć skrzynkę.
	if (fts == Field::BOX && (ftsa == Field::FLOOR || ftsa == Field::PARK))
	{
		allow_move = true;
		//Przepychamy skrzynkę.
		map[new_pp.y + dy][new_pp.x + dx] = Field::BOX;
		//Oczywiście pole na którym stała skrzynka staje się teraz podłogą.
		map[new_pp.y][new_pp.x] = Field::FLOOR;
	}
	
	if (allow_move)
	{
		//Przesuwamy gracza.
		map[player_position.y][player_position.x] = Field::FLOOR;
		player_position = new_pp;
		map[player_position.y][player_position.x] = Field::PLAYER;
	}

	//Niestety w czasie ruchu mogły „ucierpieć” miejsca na skrzynkę. ;-(
	for (auto park_position : park_positions) if (map[park_position.y][park_position.x] == Field::FLOOR) map[park_position.y][park_position.x] = Field::PARK;
}

bool Sokoban::Is_Victory() const
{
	//Tym razem dla odmiany optymistycznie zakładamy, że gracz wygrał.
	//No ale jeśli na którymkolwiek miejscu na skrzynki nie ma skrzynki to chyba założenie było zbyt optymistyczne... : -/
	for (auto park_position : park_positions) if (map[park_position.y][park_position.x] != Field::BOX) return false;
	return true;
}

int main()
{
 sf::RenderWindow window(sf::VideoMode(800, 600), "GFK Lab 01", sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
 Sokoban sokoban;
 window.setFramerateLimit(60); //limit fps do 60
 sokoban.LoadMapFromFile("plansza.txt");
 sf::Clock clock;
 sf::Font font;
 font.loadFromFile("Czcionka.ttf");
 sf::Text text;
 text.setFont(font);
 text.setCharacterSize(24);
 text.setFillColor(sf::Color::Black);
 text.setPosition(10, 10);
 sokoban.SetDrawParameters(window.getSize());
 while (window.isOpen())
 {
  sf::Event event;
  sf::Time elapsed = clock.getElapsedTime();
  text.setString("Czas gry: " + std::to_string(elapsed.asSeconds()));
  while (window.pollEvent(event))
  {
   if (event.type == sf::Event::Resized)
   {
	   float width = static_cast<float>(event.size.width);
	   float height = static_cast<float>(event.size.height);
	   window.setView(sf::View(sf::FloatRect(0, 0, width, height)));
	   sokoban.SetDrawParameters(window.getSize());
	   text.setCharacterSize(std::max(height / 30, width / 30));
   }
   if (event.type == sf::Event::Closed) window.close();
   else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) sokoban.Move_Player_Up();
   else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) sokoban.Move_Player_Down();
   else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) sokoban.Move_Player_Left();
   else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) sokoban.Move_Player_Right();
   else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) sokoban.Move_Player_Up();
   else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) sokoban.Move_Player_Down();
   else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) sokoban.Move_Player_Left();
   else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) sokoban.Move_Player_Right();   

   //Oczywiście tu powinny zostać jakoś obsłużone inne zdarzenia.Na przykład jak gracz naciśnie klawisz w lewo powinno pojawić się wywołanie metody :
   //sokoban.Move_Player_Left();
   //W dowolnym momencie mogą Państwo sprawdzić czy gracz wygrał:
   if(sokoban.Is_Victory()) {
	   sf::Sprite victory;
	   sf::Texture victory_texture;
	   victory_texture.loadFromFile("victory.png");
	   victory.setTexture(victory_texture);
	   victory.setPosition(sokoban.shift.x+window.getSize().x/4, sokoban.shift.y+window.getSize().y/8);
	   victory.setScale((window.getSize().x/victory.getGlobalBounds().width)/2, (window.getSize().y/victory.getGlobalBounds().height)/2);
	   window.draw(victory);
	   window.display();
	   sf::sleep(sf::seconds(2));
	   window.close();
   }
   if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
	   sf::Sprite lose;
	   sf::Texture lose_texture;
	   lose_texture.loadFromFile("lose.png");
	   lose.setTexture(lose_texture);
	   lose.setPosition(sokoban.shift.x+window.getSize().x/4, sokoban.shift.y+window.getSize().y/8);
	   lose.setScale((window.getSize().x/lose.getGlobalBounds().width)/2, (window.getSize().y/lose.getGlobalBounds().height)/2);
	   window.draw(lose);
	   window.display();
	   sf::sleep(sf::seconds(2));
	   window.close();
   }
  }
  window.clear(sf::Color(79, 78, 75));
  window.draw(sokoban);
  window.draw(text);
  window.display();
 }

 return 0;
}