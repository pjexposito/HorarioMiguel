#include "pebble.h"
#include "calendario.h"
#include "fpp.h"

  
#define NUM_MENU_SECTIONS 2
#define NUM_FIRST_MENU_ITEMS 2
#define NUM_SECOND_MENU_ITEMS 1
  
int loading = 0;

static Window *window;

static MenuLayer *menu_layer;

enum {
	KEY_MES,
	KEY_HORARIO,
	KEY_ERROR,
};

typedef struct {
	int index;
	char content[124];
} Tipo_horario_recibido;

static Tipo_horario_recibido horarios_recibidos[12];

/*
void process_tuple(Tuple *t)
{
    int key = t->key;
    char string_value[125];
    memset(string_value, 0, 125);
    strcpy(string_value, t->value->cstring);
    persist_write_string(key, string_value);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Escrita clave %d, con valor %s", key, string_value);
}


void in_received_handler(DictionaryIterator *iter, void *context)
{
    (void) context;
    Tuple *t = dict_read_first(iter);
    while(t != NULL)
    {
        process_tuple(t);
        t = dict_read_next(iter);
    }
    char version[20];
    time_t now = time(NULL);
    struct tm *tick_time = localtime(&now); 
    snprintf(version, 20, "Versión: %i-%i-%i",tick_time->tm_mday,(tick_time->tm_mon)+1,(tick_time->tm_year)-100); 
  
  // En 80 se guarda la fecha de actualización
  
    persist_write_string(80, version);
    vibes_short_pulse();
    loading = 0;
    layer_mark_dirty(menu_layer_get_layer(menu_layer));
}

*/

static void in_received_handler(DictionaryIterator *iter, void *context) {
	Tuple *index_tuple = dict_find(iter, KEY_MES);
	Tuple *content_tuple = dict_find(iter, KEY_HORARIO);

  
	if (index_tuple && content_tuple) {
		Tipo_horario_recibido horario;
		horario.index = index_tuple->value->int16;
		strncpy(horario.content, content_tuple->value->cstring, sizeof(horario.content) - 1);
		horarios_recibidos[horario.index] = horario;
  
      persist_write_string(horario.index, horario.content);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Grabado %s en %d", horario.content, horario.index);

	}

}


void send_int(int key, int cmd)
{
  loading = 1;
  layer_mark_dirty(menu_layer_get_layer(menu_layer));
	DictionaryIterator *iter;
 	app_message_outbox_begin(&iter);
 	Tuplet value = TupletInteger(key, cmd);
 	dict_write_tuplet(iter, &value);	
 	app_message_outbox_send();
}


static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}
  
static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return NUM_FIRST_MENU_ITEMS;
    case 1:
      return NUM_SECOND_MENU_ITEMS;
    default:
      return 0;
  }
}
  
static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      menu_cell_basic_header_draw(ctx, cell_layer, "Programas");
      break;
    case 1:
      menu_cell_basic_header_draw(ctx, cell_layer, "Herramientas");
      break;
  }
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case 0:
      switch (cell_index->row) {
        case 0:
          menu_cell_basic_draw(ctx, cell_layer, "Calendarios", "Mostrar turnos", NULL);
          break;
        case 1:
          menu_cell_basic_draw(ctx, cell_layer, "FPP", "Calculadora", NULL);
          break;                
      }
      break;
    case 1:
      switch (cell_index->row) {
        case 0:
          if (loading==0) 
            {
              char version[20];
              persist_read_string(80, version, sizeof(version));
              menu_cell_basic_draw(ctx, cell_layer, "Actualizar datos", version, NULL);
            } 
          else
            menu_cell_basic_draw(ctx, cell_layer, "Actualizando...", "Por favor, espera.", NULL);
          break;
      }
  }
}
  
  
void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case 0:
      switch (cell_index->row) {
      case 0:
        if (loading==0) carga_calendario();
        break;
      case 1:
        if (loading==0) carga_fpp();
        break;
      }
      break;
    
    case 1:
      switch (cell_index->row) {
      case 0:
         if (loading==0){
           send_int(5,5);
           char version[20];
           time_t now = time(NULL);
           struct tm *tick_time = localtime(&now); 
           snprintf(version, 20, "Versión: %i-%i-%i",tick_time->tm_mday,(tick_time->tm_mon)+1,(tick_time->tm_year)-100); 
           vibes_short_pulse();
           loading = 0;
           layer_mark_dirty(menu_layer_get_layer(menu_layer));
         }
         break;
      }
      break;    
  }
}
  
  



static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });
  
  menu_layer_set_click_config_onto_window(menu_layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));

}

static void window_unload(Window *window) {
  menu_layer_destroy(menu_layer);

}

int main(void) {
  window = window_create();
	app_message_open(192 /* inbound_size */, 100 /* outbound_size */);
  app_message_register_inbox_received(in_received_handler);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, false);

  app_event_loop();

  window_destroy(window);
}