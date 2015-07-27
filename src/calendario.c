#include "pebble.h"
#include "calendario.h"
#include "funciones.h"

  
#define MESES_TURNOS 20

#define LINEA_HORIZONTAL_INICIAL 33
#define LINEA_HORIZONTAL 17

#define LINEA_VERTICAL_INICIAL 2
#define LINEA_VERTICAL 20

#ifdef PBL_COLOR
#   define COLOR_PRINCIPAL GColorBlack  // El color del lápiz es blanco
#   define COLOR_FONDO GColorWhite       // y el fondo, azul para BASALT
#   define COLOR_M GColorMediumAquamarine     
#   define COLOR_T GColorJaegerGreen
#   define COLOR_AA GColorMagenta   
#   define COLOR_AT GColorVividViolet   
#   define COLOR_L GColorChromeYellow  
#   define COLOR_FA GColorPictonBlue 
#   define COLOR_FT GColorVividCerulean 
#   define COLOR_D GColorLightGray  
#   define COLOR_LINEAS GColorDarkGray 
#   define COLOR_NOMBREDIAS GColorBlack 

#else
#   define COLOR_PRINCIPAL GColorBlack  // El color del lápiz es blanco
#   define COLOR_FONDO GColorWhite  // y el fondo, negro
#   define COLOR_M GColorWhite  
#   define COLOR_T GColorWhite
#   define COLOR_AA GColorWhite  
#   define COLOR_AT GColorWhite  
#   define COLOR_L GColorWhite  
#   define COLOR_FA GColorWhite 
#   define COLOR_FT GColorWhite 
#   define COLOR_D GColorWhite 
#   define COLOR_LINEAS GColorBlack
#   define COLOR_NOMBREDIAS GColorBlack 

#endif  
  

#define FUENTE FONT_KEY_GOTHIC_14
#define FUENTE_BOLD FONT_KEY_GOTHIC_14_BOLD

#define FUENTE_GRANDE FONT_KEY_GOTHIC_24
#define FUENTE_GRANDE_BOLD FONT_KEY_GOTHIC_24_BOLD
  
// La variable chkturnos puede tener valor 1 si se muestra el calendario de turnos y valor 0 si se muestra el de días.
// Se define de forma global las variables día, mes y año (dado que pueden cambiar a lo largo de la ejecución)
// Mes_actual y dia_actual siempre guardarán el valor del día y el mes en el que se ejecuta el programa (no varían)
int dia, mes, ano, mes_actual, dia_actual, chkturnos;

int turnos[20][33];


int cargando=0;

struct Fecha{
    int dia, mes, ano;
};

// Matriz básica para transformar el número de mes en el nombre del mes.
static const char *nombre_mes[13] =
{ "vacio", "enero", "febrero", "marzo", "abril", "mayo", "junio", "julio", "agosto", "septiembre", "octubre",
    "noviembre", "diciembre" };



// Ventana principal
static Window *window;


//Capas del reloj
Layer *CapaLineas; // La capa principal donde se dibuja el calendario




// Este código no es mío, así que poco puedo comentar. El caso es que funciona perfectamente.
// Sacado de: http://www.codecodex.com/wiki/Calculate_the_number_of_days_in_a_month#C.2FC.2B.2B  
int numero_de_dias(int month, int year)
  {
  
  int numberOfDays;  
  if (month == 4 || month == 6 || month == 9 || month == 11)  
    numberOfDays = 30;  
  else if (month == 2)  
  { 
    bool isLeapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);  
    if (isLeapYear)  
      numberOfDays = 29;  
    else  
      numberOfDays = 28;  
  }  
  else  
    numberOfDays = 31; 
  return numberOfDays;
  }

// De nuevo, este código no es mío. Tan sólo he hecho una pequeña variación sobre el código encontrado
// en: http://stackoverflow.com/questions/15127615/determining-day-of-the-week-using-zellers-congruence
int dweek(int year, int month, int day)
   {
     int h,q,m,k,j;
  static int conv[] = {6,7,1,2,3,4,5,6};
    if(month == 1)
    {
    month = 13;
    year--;
    }
    if (month == 2)
    {
      month = 14;
      year--;
    }
    q = day;
    m = month;
    k = year % 100;
    j = year / 100;
    h = q + 13*(m+1)/5 + k + k/4 + j/4 + 5*j;
    h = h % 7;
    return conv[h];
   }

// Otro código que no es mio. Es de http://stackoverflow.com/questions/19377396/c-get-day-of-year-from-date
int yisleap(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int get_yday(int mon, int day, int year)
{
    static const int days[2][13] = {
        {0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
        {0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
    };
    int leap = yisleap(year);

    return days[leap][mon] + day;
}


// Función chapucera creada por mi. Devuelve un struct de tipo Fecha al darle un año y los dias
struct Fecha devuelve_fecha(ano, dias)
{
    struct Fecha valor_retorno;
    int days_in_month[13] = { 0, 31, 28+yisleap(ano), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int x, mes, dia, suma=0;
    for (x=1;x<14;x++)
    {
      suma = suma+days_in_month[x];
      if (suma >= dias)
        {
        mes = x;
        dia = -(suma-days_in_month[x]-dias);
        x = 14;
        }
    }
    valor_retorno.dia = dia;
    valor_retorno.mes = mes;
    valor_retorno.ano = ano;
    return valor_retorno;
}


void anade_datos(const char* input, int mes)
{
       //APP_LOG(APP_LOG_LEVEL_DEBUG, "Recibo %s con mes %i", input, mes);

    int y = 2;
    char dest[6];
    int dias;
    memset(dest, 0, 6);
    subString (input, 0, 2, dest);
    dias = atoi(dest);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Dias %s. En número es %i", dest, atoi(dest));
    
    memset(dest, 0, 6);
    subString (input, 2, 4, dest);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Año %s", dest);
    turnos[mes][0]=atoi(dest);
    
    memset(dest, 0, 6);
    subString (input, 6, 2, dest);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Mes %s", dest);
    turnos[mes][1]=atoi(dest);
    
    
    for (int x=8;x<dias+8;x++)
    {
        memset(dest, 0, 6);
        subString (input, x, 1, dest);
        turnos[mes][y]=atoi(dest);
        y++;
    }
}


void carga_datos()
{
    
    char username[64];
    persist_read_string(0, username, sizeof(username));
    if (strcmp(username, "")==0)
    {
        chkturnos=1;
    }
    else
    {
      for (int x=0;x<MESES_TURNOS;x++)
      {
        memset(username, 0, 64);
        persist_read_string(x, username, sizeof(username));
        if (strcmp(username, "")!=0)
          {
          anade_datos(username, x);
          //APP_LOG(APP_LOG_LEVEL_DEBUG, "%s", username);
          }
      }

    }
}



// Esta función se ejecutará cada vez que se refresque la CapaLineas
void CapaLineas_update_callback(Layer *me, GContext* ctx)
{

    // left, top, anchura, altura
    // 144x168
    
    // Color del fondo y color del trazo
        
    graphics_context_set_stroke_color(ctx, COLOR_PRINCIPAL);
    graphics_context_set_fill_color(ctx, COLOR_PRINCIPAL);
    graphics_context_set_text_color(ctx, COLOR_PRINCIPAL);  

    // Se pinta los carácteres fijos
    if (chkturnos)
    {  
      graphics_draw_text(ctx, "10", fonts_get_system_font(FUENTE), GRect(18, -2, 12, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
      graphics_draw_text(ctx, "15", fonts_get_system_font(FUENTE), GRect(58, -2, 12, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
      graphics_draw_text(ctx, "20", fonts_get_system_font(FUENTE), GRect(99, -2, 12, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
      graphics_draw_text(ctx, "00", fonts_get_system_font(FUENTE), GRect(130, -2, 12, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    }
    else
    {
      char temp_fecha[30];
      snprintf(temp_fecha, 30, "%s de %d",nombre_mes[mes],ano);
      graphics_draw_text(ctx, temp_fecha, fonts_get_system_font(FUENTE), GRect(1, -3, 140, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    }
    graphics_draw_text(ctx, "Lun", fonts_get_system_font(FUENTE), GRect(0, 10, 23, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    graphics_draw_text(ctx, "Mar", fonts_get_system_font(FUENTE), GRect(0, 32, 23, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    graphics_draw_text(ctx, "Mie", fonts_get_system_font(FUENTE), GRect(0, 54, 23, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    graphics_draw_text(ctx, "Jue", fonts_get_system_font(FUENTE), GRect(0, 76, 23, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    graphics_draw_text(ctx, "Vie", fonts_get_system_font(FUENTE), GRect(0, 98, 23, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    graphics_draw_text(ctx, "Sab", fonts_get_system_font(FUENTE), GRect(0, 120, 23, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    graphics_draw_text(ctx, "Dom", fonts_get_system_font(FUENTE), GRect(0, 142, 23, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);



  
  
    // Pinta el nombre del mes
    //graphics_draw_text(ctx, nombre_mes[mes], fonts_get_system_font(FUENTE_GRANDE_BOLD), GRect(0, 5, 72, 10), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
        
    // Pasa el int del año a letra y pinta el año
   
  
  /*
    char temp[20]  = "";
    snprintf(temp, sizeof(temp), "%s %d", nombre_mes[mes],ano);
    graphics_draw_text(ctx, temp, fonts_get_system_font(FUENTE_GRANDE_BOLD), GRect(1, 2, 143, 10), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  */    

    // nueva_fila indica si el mes tiene 5 o 6 filas. Si es igual a 0, tiene 5, si es 1, tiene 6.
        

    // Lineas horizontales
    for (int x=0; x<13; x++)
        {
        graphics_fill_rect(ctx, GRect(0, 13+(x*22), 144, 1), 0, GCornerNone);
        graphics_fill_rect(ctx, GRect(24, 23+(x*22), 144, 1), 0, GCornerNone);
        }
        
    // Lineas verticales
    for (int x=0; x<15;x++)
        graphics_fill_rect(ctx, GRect(24+(x*8), 13, 1, 155), 0, GCornerNone);

    
  //Si se quiere usar los datos en bruto:
  // ******************************
  // ******************************
  /*
    int ano = 2015;
    int dias = 208;
    struct fecha a;
    a = devuelve_fecha(ano, dias);
    printf("Dia: %d, mes %d, ano: %d\n", a.dia, a.mes, a.ano);
    
    // Al contrario es:
    printf("Es el dia: %d\n", get_yday(a.mes, a.dia, a.ano));
   */
  
  
  // Se pintan los datos variables
  
  // Los días de la semana
  // mes, ano, dia_actual
  int dia_semana = dweek(ano,mes,dia);
  int lunes = dia-(dia_semana-1);

  for (int x=0; x<7; x++)
      {
      int dia_a_pintar = lunes +x;
      if (dia_a_pintar > numero_de_dias(mes,ano)) 
        dia_a_pintar = dia_a_pintar - numero_de_dias(mes,ano);
      if (dia_a_pintar < 1)
        dia_a_pintar = numero_de_dias(mes-1,ano) - dia_a_pintar;
      char temp_dia[4];
      snprintf(temp_dia, 4, "%i",dia_a_pintar);
      graphics_draw_text(ctx, temp_dia, fonts_get_system_font(FUENTE), GRect(0, 20+(x*22), 23, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
      }  
  
  // Los horarios en las celdas
      graphics_context_set_fill_color(ctx, GColorWhite );
      for (int x=0; x<13; x++)
      {
      graphics_fill_rect(ctx,GRect(25, 24+(x*22), 138, 11),0,GCornerNone );
      graphics_draw_text(ctx, "10:00-15:00", fonts_get_system_font(FUENTE), GRect(24, 20+(x*22), 90, 7), GTextOverflowModeFill , GTextAlignmentLeft, NULL);
      graphics_draw_text(ctx, "21:00-00:30", fonts_get_system_font(FUENTE), GRect(85, 20+(x*22), 100, 7), GTextOverflowModeFill , GTextAlignmentLeft, NULL);

        //graphics_draw_text(ctx, "21:00-00:30", fonts_get_system_font(FUENTE), GRect(13, 20+(x*22), 143, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

      }  
  
  // Y los cuadros en los horarios
      graphics_context_set_fill_color(ctx, GColorBlack );
      // El 0 equivale a las 10, 1 = 11, 2 = 12, 3 = 13, 4 = 14, 5 = 15, 6 = 16, 7 = 17, 8 = 18, 9 = 19
      // 10 = 20, 11 = 21, 12 = 22, 13 = 23, 14 = 00, 15 = 01
    
      /*
      RANGO COMPLETO
      
      for (int x=0; x<15; x++)
      {  
      graphics_fill_rect(ctx,GRect(25+(x*8), 14, 7, 9),0,GCornerNone );
      }
      */
  
  
  // EJEMPLOS
      for (int x=0; x<5; x++)
      {  
      graphics_fill_rect(ctx,GRect(25+(x*8), 14, 7, 9),0,GCornerNone );
      }
      for (int x=11; x<15; x++)
      {  
      graphics_fill_rect(ctx,GRect(25+(x*8), 14, 7, 9),0,GCornerNone );
      }  
        
      for (int x=8; x<11; x++)
      {  
      graphics_fill_rect(ctx,GRect(25+(x*8), 14+22, 7, 9),0,GCornerNone );
      }  
      for (int x=1; x<4; x++)
      {  
      graphics_fill_rect(ctx,GRect(25+(x*8), 14+44, 7, 9),0,GCornerNone );
      }    
  
  // FIN DE EJEMPLO
      for (int x=0; x<7; x++)
      { 
      graphics_fill_rect(ctx,GRect(25, 14+(x*22), 7, 9),0,GCornerNone );
      }  
      //graphics_fill_rect(ctx,GRect(25, 36, 7, 9),0,GCornerNone );
  
}  // Y termina la función

// Se pulsa el botón arriba
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    for (int x=0; x<7;x++)
    {
      dia = dia-1; 
      if (dia < 1) 
      {
        mes = mes-1;
        dia = numero_de_dias(mes,ano);
      }
    }
    if (mes==0)
    {
        mes = 12;
        ano--;
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "%d/%d/%d. Dia del año: %d", dia, mes, ano, get_yday(mes, dia, ano));

    layer_mark_dirty(CapaLineas);
    // Se resta un mes al actual y si el mes es inferior a 1, se resta un año
}


// Se pulsa el botón select
void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    if (chkturnos==1)
        chkturnos=0;
    else
        chkturnos=1;
    layer_mark_dirty(CapaLineas);

    // Se usa el select para cambiar entre calendario normal y de turnos
}

// Se pulsa el botón abajo
void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    for (int x=0; x<7;x++)
    {
      dia = dia+1; 
      if (dia > numero_de_dias(mes,ano)) 
      {
        dia = 1;
        mes = mes+1;
      }
    }
    if (mes==13)
    {
        mes = 1;
        ano++;
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "%d/%d/%d. Dia del año: %d", dia, mes, ano, get_yday(mes, dia, ano));

    layer_mark_dirty(CapaLineas);
    // Se suma un mes al actual, y si supera el 12, se suma un año
}

// Se definen las funciones asociadas a la pulsación de botones
void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

void window_load(Window *window)
{
    //funcion para saber el día, el mes y el año actual
 
    time_t now = time(NULL);
    struct tm *tick_time = localtime(&now); 
    dia_actual = tick_time->tm_mday;
    dia = dia_actual;
    mes = tick_time->tm_mon+1;
    mes_actual = mes;
    ano = tick_time->tm_year+1900;
    
    // Se establece chkturnos a 0 para mostrar el mes y el año. Si es 1, se muestran horas en la zona superior
    chkturnos=0;
    
    // Línea de DEBUG, por si acaso. Debe estar desactivada siempre que sea posible
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "El primer dia del mes %i, del año %i es %i", mes, ano, dweek(ano,mes,dia));
    carga_datos();
}

static void window_unload(Window *window)
{
    
    //Al cerrar la aplicación, matamos las capas y desactivamos los procesos
    
    layer_destroy(CapaLineas);
    
    // Y por último se borra de la memoria la ventana principal
    window_destroy(window);
}
  
void carga_calendario()
{
    window = window_create();
  	WindowHandlers handlers = {
		.load = window_load,
		.unload = window_unload
	  };
  
  	window_set_window_handlers(window, (WindowHandlers) handlers);

    window_set_click_config_provider(window, click_config_provider);
    
    
    window_stack_push(window, true /* Animado */);
    window_set_background_color(window, COLOR_FONDO);
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    
    //Se añade la CapaLineas
    CapaLineas = layer_create(bounds);
    layer_set_update_proc(CapaLineas, CapaLineas_update_callback); 
    layer_add_child(window_layer, CapaLineas); 
    
    

}




