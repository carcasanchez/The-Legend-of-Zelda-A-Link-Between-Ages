#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1FileSystem.h"
#include "j1Textures.h"
#include "j1Map.h"
#include "j1Input.h"
#include "j1Pathfinding.h"
#include "j1GameLayer.h"
#include "j1EntityManager.h"
#include "Object.h"
#include "Player.h"
#include <math.h>

j1Map::j1Map() : j1Module(), map_loaded(false)
{
	name = ("map");
	data = &bigData;
	map_file = &normal_map_file;
}

// Destructor
j1Map::~j1Map()
{}

// Called before render is available
bool j1Map::Awake(pugi::xml_node& config)
{
	LOG("Loading Map Parser");
	bool ret = true;

	folder = config.child("folder").child_value();

	return ret;
}

bool j1Map::Start()
{
	debug_tex = App->tex->Load("maps/path_debug.png");
	return true;
}

bool j1Map::Update(float dt)
{
	if (App->game->debug && App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
	{
		debug_collisions = !debug_collisions;
	}



	return true;
}

void j1Map::Draw()
{
	if(map_loaded == false)
		return;

	
	SDL_Rect cameraSection;

	cameraSection.x = (App->render->renderZone.x / data->tile_width);
	cameraSection.y = (App->render->renderZone.y / data->tile_height);
	cameraSection.w = ((App->render->renderZone.w) / data->tile_width) + 1;
	cameraSection.h = ((App->render->renderZone.h) / data->tile_height) + 2;
	
	for(list<MapLayer*>::iterator item = data->layers.begin(); item != data->layers.cend(); item++)
	{
		MapLayer* layer = (*item);

		if(layer->properties.Get("Nodraw") != 0 && layer->properties.Get("Player")!=0 && debug_collisions == false)
			continue;

		if (layer->properties.Get("Enemy") )
			continue;

		if (layer->properties.Get("Paintover") || layer->properties.Get("Bush") || layer->properties.Get("Jump") || layer->properties.Get("Projectile"))
			continue;
					
		for (int x = cameraSection.x; x < cameraSection.x + cameraSection.w; x++)
		{
			for (int y = cameraSection.y; y < cameraSection.y + cameraSection.h; y++)
			{
				int tile_id = 0;

				if (x > data->width || y > data->height)
					tile_id = 0;
				else tile_id = layer->Get(x, y);

				if (tile_id > 0)
				{
					iPoint pos = MapToWorld(x, y);
					TileSet* tileset = GetTilesetFromTileId(tile_id);
					SDL_Rect r = tileset->GetTileRect(tile_id);
					App->render->Blit(tileset->texture, pos.x, pos.y, &r);
				}
			}

		}
				
		

	}

	/*iPoint a = App->game->em->player->jumpOrigin;
	iPoint b = App->game->em->player->toJump;
	
	if (!b.IsZero())
	{
		a = App->map->MapToWorld(a.x, a.y);
		b = App->map->MapToWorld(b.x, b.y);
		App->render->DrawLine(a.x, a.y, b.x, b.y, 255, 255, 255, 255);
	}*/
}

void j1Map::DrawOver()
{
	if (map_loaded == false)
		return;


	SDL_Rect cameraSection;

	cameraSection.x = (App->render->renderZone.x / data->tile_width);
	cameraSection.y = (App->render->renderZone.y / data->tile_height);
	cameraSection.w = ((App->render->renderZone.w) / data->tile_width) + 1;
	cameraSection.h = ((App->render->renderZone.h) / data->tile_height) + 2;

	for (list<MapLayer*>::iterator item = data->layers.begin(); item != data->layers.cend(); item++)
	{
		MapLayer* layer = (*item);

		if (layer->properties.Get("Paintover") == 0)
			continue;

		for (int x = cameraSection.x; x < cameraSection.x + cameraSection.w; x++)
		{
			for (int y = cameraSection.y; y < cameraSection.y + cameraSection.h; y++)
			{

				int tile_id = 0;

				if (x > data->width || y > data->height)
					tile_id = 0;
				else tile_id = layer->Get(x, y);

				if (tile_id > 0)
				{
					iPoint pos = MapToWorld(x, y);
					TileSet* tileset = GetTilesetFromTileId(tile_id);
					SDL_Rect r = tileset->GetTileRect(tile_id);
					App->render->Blit(tileset->texture, pos.x, pos.y, &r);
				}
			}

		}

		break;
	}

}

int Properties::Get(const char* value, int default_value) const
{
	
	for (std::list<Property*>::const_iterator item = list.begin(); item != list.cend(); item++)
	{
		if ((*item)->name == value)
			return (*item)->value;
	}

	return default_value;
}

TileSet* j1Map::GetTilesetFromTileId(int id) const
{
	
	std::list<TileSet*>::const_iterator item = data->tilesets.begin();
	TileSet* set = (*item);

	for (; item != data->tilesets.cend(); item++)
	{
		if (id < (*item)->firstgid)
		{
			set = (item._Ptr->_Prev->_Myval); 
			break;
		}
		set = (*item);
	}

	return set;
}

iPoint j1Map::MapToWorld(int x, int y) const
{
	iPoint ret;

	if(data->type == MAPTYPE_ORTHOGONAL)
	{
		ret.x = x * data->tile_width;
		ret.y = y * data->tile_height;
	}
	else if(data->type == MAPTYPE_ISOMETRIC)
	{
		ret.x = (x - y) * (data->tile_width * 0.5f);
		ret.y = (x + y) * (data->tile_height * 0.5f);
	}
	else
	{
		LOG("Unknown map type");
		ret.x = x; ret.y = y;
	}

	return ret;
}

iPoint j1Map::WorldToMap(int x, int y) const
{
	iPoint ret(0,0);

	if(data->type == MAPTYPE_ORTHOGONAL)
	{
		ret.x = (x / data->tile_width);
		ret.y = (y / data->tile_height);
	}
	else if(data->type == MAPTYPE_ISOMETRIC)
	{
		
		float half_width = data->tile_width * 0.5f;
		float half_height = data->tile_height * 0.5f;
		ret.x = int( (x / half_width + y / half_height) / 2);
		ret.y = int( (y / half_height - (x / half_width)) / 2);
	}
	else
	{
		LOG("Unknown map type");
		ret.x = x; ret.y = y;
	}

	return ret;
}

iPoint j1Map::GetTileCenter(iPoint tile)
{
	iPoint ret = MapToWorld(tile.x, tile.y);

	ret.x += data->tile_width/2;
	ret.y += data->tile_height/2;
	return  ret ;
}

iPoint j1Map::WorldToMapMouse(int x, int y) const
{
	iPoint ret(0, 0);

	if (data->type == MAPTYPE_ORTHOGONAL)
	{
		ret.x = x / data->tile_width;
		ret.y = y / data->tile_height;
	}
	else if (data->type == MAPTYPE_ISOMETRIC)
	{

		float half_width = data->tile_width * 0.5f;
		float half_height = data->tile_height * 0.5f;
		ret.x = int((x / half_width + y / half_height) / 2) - 1;
		ret.y = int((y / half_height - (x / half_width)) / 2);
	}
	else
	{
		LOG("Unknown map type");
		ret.x = x; ret.y = y;
	}

	return ret;
}


SDL_Rect TileSet::GetTileRect(int id) const
{
	int relative_id = id - firstgid;
	SDL_Rect rect;
	rect.w = tile_width;
	rect.h = tile_height;
	rect.x = margin + ((rect.w + spacing) * (relative_id % num_tiles_width));
	rect.y = margin + ((rect.h + spacing) * (relative_id / num_tiles_width));
	return rect;
}

TileSet::~TileSet()
{
	App->tex->UnLoad(texture);
}

// Called before quitting
bool j1Map::CleanUp()
{
	LOG("Unloading map");

	// Remove all tilesets
	for (std::list<TileSet*>::iterator item = smallData.tilesets.begin(); item != smallData.tilesets.cend(); item++)
	{
		RELEASE_ARRAY(*item);
	}
	for (std::list<TileSet*>::iterator item = bigData.tilesets.begin(); item != bigData.tilesets.cend(); item++)
		RELEASE(*item);

	smallData.tilesets.clear();
	bigData.tilesets.clear();

	// Remove all layers
	for (std::list<MapLayer*>::iterator item = smallData.layers.begin(); item != smallData.layers.cend(); item++)
		RELEASE_ARRAY(*item);
	for (std::list<MapLayer*>::iterator item = bigData.layers.begin(); item != bigData.layers.cend(); item++)
		RELEASE(*item);

	smallData.layers.clear();
	bigData.layers.clear();

	// Clean up the pugui tree
	small_map_file.reset();
	normal_map_file.reset();

	return true;
}

// Load new map
bool j1Map::Load(const char* file_name)
{
	bool ret = true;
	string tmp = folder.c_str();
	tmp += file_name;

	char* buf;
	int size = App->fs->Load(tmp.c_str(), &buf);
	pugi::xml_parse_result result = map_file->load_buffer(buf, size);

	RELEASE_ARRAY(buf);

	if(result == NULL)
	{
		LOG("Could not load map xml file %s. pugi error: %s", file_name, result.description());
		ret = false;
	}
	else
	{
		ret = LoadMap();
	}

	// Load all tilesets info ----------------------------------------------
	pugi::xml_node tileset;
	for(tileset = map_file->child("map").child("tileset"); tileset && ret; tileset = tileset.next_sibling("tileset"))
	{
		TileSet* set = new TileSet();

		if(ret == true)
		{
			ret = LoadTilesetDetails(tileset, set);
		}

		if(ret == true)
		{
			ret = LoadTilesetImage(tileset, set);
		}

		data->tilesets.push_back(set);
	}

	// Load layer info ----------------------------------------------
	pugi::xml_node layer;
	for(layer = map_file->child("map").child("layer"); layer && ret; layer = layer.next_sibling("layer"))
	{
		MapLayer* lay = new MapLayer();

		ret = LoadLayer(layer, lay);

		if(ret == true)
			data->layers.push_back(lay);
	}

	if(ret == true)
	{
		LOG("Successfully parsed map XML file: %s", file_name);
		LOG("width: %d height: %d", data->width, data->height);
		LOG("tile_width: %d tile_height: %d", data->tile_width, data->tile_height);

		
		for (std::list<TileSet*>::iterator item = data->tilesets.begin(); item != data->tilesets.cend(); item++)
		{
			TileSet* s = *item;
			LOG("Tileset ----");
			//LOG("name: %s firstgid: %d", s->name.GetString(), s->firstgid);
			LOG("tile width: %d tile height: %d", s->tile_width, s->tile_height);
			LOG("spacing: %d margin: %d", s->spacing, s->margin);
		}

		for (std::list<MapLayer*>::iterator item = data->layers.begin(); item != data->layers.cend(); item++)
		{

			//Create Entityes based on map data
			if ((*item)->properties.Get("Bush"))
				for (int x = 0; x < data->width; x++)
					for (int y = 0; y < data->height; y++)
					{
						int tile_id = 0;

						if (x > data->width || y > data->height)
							tile_id = 0;
						else tile_id = (*item)->Get(x, y);

						if (tile_id > 0)
						{
							App->game->em->CreateObject(1, x, y, BUSH);
						}
					}	
			
		}
				
	}

	map_loaded = ret;

	return ret;
}

// Load map general properties
bool j1Map::LoadMap()
{
	bool ret = true;
	pugi::xml_node map = map_file->child("map");

	if(map == NULL)
	{
		LOG("Error parsing map xml file: Cannot find 'map' tag.");
		ret = false;
	}
	else
	{
		data-> width = map.attribute("width").as_int();
		data-> height = map.attribute("height").as_int();
		data-> tile_width = map.attribute("tilewidth").as_int();
		data-> tile_height = map.attribute("tileheight").as_int();
		string bg_color = map.attribute("backgroundcolor").as_string();

		data->background_color.r = 0;
		data->background_color.g = 0;
		data->background_color.b = 0;
		data->background_color.a = 0;

		if(bg_color.length() > 0)
		{
			string red, green, blue;
			
			bg_color += red;
			bg_color += green;
			bg_color += blue;
			

			int v = 0;

			sscanf_s(red.c_str(), "%x", &v);
			if(v >= 0 && v <= 255) data->background_color.r = v;

			sscanf_s(green.c_str(), "%x", &v);
			if(v >= 0 && v <= 255) data->background_color.g = v;

			sscanf_s(blue.c_str(), "%x", &v);
			if(v >= 0 && v <= 255) data->background_color.b = v;
		}

		string orientation = map.attribute("orientation").as_string();

		if(orientation == "orthogonal")
		{
			data->type = MAPTYPE_ORTHOGONAL;
		}
		else if(orientation == "isometric")
		{
			data->type = MAPTYPE_ISOMETRIC;
		}
		else if(orientation == "staggered")
		{
			data->type = MAPTYPE_STAGGERED;
		}
		else
		{
			data->type = MAPTYPE_UNKNOWN;
		}
	}

	return ret;
}

bool j1Map::LoadTilesetDetails(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	set->name = tileset_node.attribute("name").as_string();
	set->firstgid = tileset_node.attribute("firstgid").as_int();
	set->tile_width = tileset_node.attribute("tilewidth").as_int();
	set->tile_height = tileset_node.attribute("tileheight").as_int();
	set->margin = tileset_node.attribute("margin").as_int();
	set->spacing = tileset_node.attribute("spacing").as_int();
	pugi::xml_node offset = tileset_node.child("tileoffset");

	if(offset != NULL)
	{
		set->offset_x = offset.attribute("x").as_int();
		set->offset_y = offset.attribute("y").as_int();
	}
	else
	{
		set->offset_x = 0;
		set->offset_y = 0;
	}

	return ret;
}

bool j1Map::LoadTilesetImage(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	pugi::xml_node tmp = tileset_node;
	pugi::xml_node image = tileset_node.child("image");

	if(image == NULL)
	{
		LOG("Error parsing tileset xml file: Cannot find 'image' tag.");
		ret = false;
	}
	else
	{
		set->texture = App->tex->Load(PATH(folder.c_str(), image.attribute("source").as_string()));
		int w, h;
		SDL_QueryTexture(set->texture, NULL, NULL, &w, &h);
		set->tex_width = image.attribute("width").as_int();

		if(set->tex_width <= 0)
		{
			set->tex_width = w;
		}

		set->tex_height = image.attribute("height").as_int();

		if(set->tex_height <= 0)
		{
			set->tex_height = h;
		}

		set->num_tiles_width = set->tex_width / set->tile_width;
		set->num_tiles_height = set->tex_height / set->tile_height;
	}

	return ret;
}

bool j1Map::LoadLayer(pugi::xml_node& node, MapLayer* layer)
{
	bool ret = true;

	layer->name = node.attribute("name").as_string();
	layer->width = node.attribute("width").as_int();
	layer->height = node.attribute("height").as_int();
	LoadProperties(node, layer->properties);
	pugi::xml_node layer_data = node.child("data");

	if(layer_data == NULL)
	{
		LOG("Error parsing map xml file: Cannot find 'layer/data' tag.");
		ret = false;
		RELEASE_ARRAY(layer);
	}
	else
	{
		
		string lol = layer_data.first_child().value();

		int bookmark = 1;
		for (int i = 0; i < (layer->width * layer->height); i++)
		{
			int coma = lol.find_first_of(",", bookmark);
			string temp = lol.substr(bookmark, (coma-bookmark)).c_str();
			layer->data.push_back(atoi(temp.c_str()));
			bookmark = coma + 1;
		}

		int i = 0;
		for(pugi::xml_node tile = layer_data.child("tile"); tile; tile = tile.next_sibling("tile"))
		{
			layer->data.push_back(tile.attribute("gid").as_int(0));
		}
	}






	return ret;
}

// Load a group of properties from a node and fill a list with it
bool j1Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
	bool ret = false;

	pugi::xml_node data = node.child("properties");

	if(data != NULL)
	{
		pugi::xml_node prop;

		for(prop = data.child("property"); prop; prop = prop.next_sibling("property"))
		{
			Properties::Property* p = new Properties::Property();

			p->name = prop.attribute("name").as_string();
			p->value = prop.attribute("value").as_int();

			properties.list.push_back(p);
		}
	}

	return ret;
}

bool j1Map::CreateWalkabilityMap(int& width, int& height, vector<int>& player_buffer, vector<int>& enemy_buffer) const
{
	bool ret = false;
	
	
	for(std::list<MapLayer*>::const_iterator item = data->layers.begin(); item != data->layers.cend(); item++)
	{
		MapLayer* layer = *item;
		
		//Player map
		if (layer->properties.Get("Navigation", 0) && layer->properties.Get("Player", 0))
		{		

			for (int y = 0; y < data->height; ++y)
			{
				for (int x = 0; x < data->width; ++x)
				{
					
					int tile_id = layer->Get(x, y);
					TileSet* tileset = (tile_id > 0) ? GetTilesetFromTileId(tile_id) : NULL;

					if (tileset != NULL)
					{
						player_buffer.push_back(tile_id);
					}
					else player_buffer.push_back(0);
				}
			}

			width = data->width;
			height = data->height;
			ret = true;
		}
			
		//Enemy_map
		if (layer->properties.Get("Navigation", 0) && layer->properties.Get("Enemy", 0))
		{

			for (int y = 0; y < data->height; ++y)
			{
				for (int x = 0; x < data->width; ++x)
				{
					int tile_id = layer->Get(x, y);
					TileSet* tileset = (tile_id > 0) ? GetTilesetFromTileId(tile_id) : NULL;

					if (tileset != NULL)
					{
						enemy_buffer.push_back(tile_id);
					}
					else enemy_buffer.push_back(0);
				}
			}
			width = data->width;
			height = data->height;
			ret = true;
		}
		
	}

	return ret;
}

bool j1Map::LoadSmallDataMap(const char* file)
{
	data = &smallData;
	map_file = &small_map_file;
	return Load(file);
}


void j1Map::UnloadSmallDataMap()
{
	UnLoadData();
	data = &bigData;
	map_file = &normal_map_file;
}

void j1Map::DebugPath(vector<iPoint> path)
{
	if(debug_path)
		for (int i = 0, j = path.size(); i < j; i++)
		{
			iPoint tilePos = MapToWorld(path[i].x, path[i].y);
			App->render->Blit(debug_tex, tilePos.x, tilePos.y);
		}
}

void j1Map::UnLoadData()
{
	// Remove all tilesets
	for (std::list<TileSet*>::iterator item = data->tilesets.begin(); item != data->tilesets.cend(); item++)
		RELEASE_ARRAY(*item);

	data->tilesets.clear();

	// Remove all layers
	for (std::list<MapLayer*>::iterator item = data->layers.begin(); item != data->layers.cend(); item++)
		RELEASE_ARRAY(*item);

	data->layers.clear();


	// Clean up the pugui tree
	map_file->reset();
}



