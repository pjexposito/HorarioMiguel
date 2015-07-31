#include "pebble.h"
#include "calendario.h"
#include "funciones.h"

#define PERSIST_KEY_DATOS 10


  
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

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint;

int dia, mes, ano, mes_actual, dia_actual, chkturnos, num_dia, num_dia_actual;

int turnos[20][33];


int cargando=0;

struct Fecha{
    int dia;
    int mes; 
    int ano;
};

struct Horario{
    int hora_inicio;
    int hora_fin; 
    int minuto_inicio;
    int minuto_fin;
    int hora2_inicio;
    int hora2_fin; 
    int minuto2_inicio;
    int minuto2_fin;
};

// Matriz básica para transformar el número de mes en el nombre del mes.
static const char *nombre_mes[13] =
{ "vacio", "enero", "febrero", "marzo", "abril", "mayo", "junio", "julio", "agosto", "septiembre", "octubre",
    "noviembre", "diciembre" };



// Para un horario como 10:00-13:30  17:00-01:00 sería algo así
//                      0 14  28 60
// Ahora sumo 48 a cada número, siendo 0, por ejemplo, 48 para que equivalga a 0
//                      48 62  76 108
// Quedando de ese modo  0 > L l, o quitando espacios, como 0>Ll 


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
struct Fecha devuelve_fecha(int ano, int dias)
{
    struct Fecha valor_retorno;
    int days_in_month[13] = { 0, 31, 28+yisleap(ano), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    int x, mes=1, dia=1, suma=0;
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


char * hex2bin(char * hex)
{
	// your code goes here
    int resta;
    const char binary[16][5] = {"0000", "0001", "0010", "0011", "0100", "0101","0110", "0111", "1000", "1001", "1010", "1011", "1100", "1101", "1110","1111"};
    static char bin[16];
    char  a;
    int x=0;
    do {
        a = hex[x];
        resta = (a > 64) ? 55 : 48;
        if (x == 0) 
            strcpy(bin, binary[a-resta]);
        else
            strcat(bin, binary[a-resta]);
        x++;
    } while (hex[x] != '\0');
	return bin;
}


struct Horario devuelve_horario(char * clave_horario) {
  static const char *todos_los_horarios[64] =
  { "10:00", "10:15", "10:30", "10:45", "11:00", "11:15", "11:30", "11:45", "12:00", "12:15", "12:30", "12:45", 
    "13:00", "13:15", "13:30", "13:45", "14:00", "14:15", "14:30", "14:45", "15:00", "15:15", "15:30", "15:45", 
    "16:00", "16:15", "16:30", "16:45", "17:00", "17:15", "17:30", "17:45", "18:00", "18:15", "18:30", "18:45", 
    "19:00", "19:15", "19:30", "19:45", "20:00", "20:15", "20:30", "20:45", "21:00", "21:15", "21:30", "21:45", 
    "22:00", "22:15", "22:30", "22:45", "23:00", "23:15", "23:30", "23:45", "00:00", "00:15", "00:30", "00:45", 
    "01:00", "L", "99:99", "21:00" /*Vacio*/};
  struct Horario valor_devuelto;
  char dest[2];
  static char cadena[24];
  strcpy(cadena, todos_los_horarios[(clave_horario[0]-48)]);
  strcat(cadena, "-");
  strcat(cadena, todos_los_horarios[(clave_horario[1]-48)]);
  strcat(cadena, " ");
  strcat(cadena, todos_los_horarios[(clave_horario[2]-48)]);
  strcat(cadena, "-");
  strcat(cadena, todos_los_horarios[(clave_horario[3]-48)]);

  memset(dest, 0, 2);
  subString (cadena, 0, 2, dest);
  valor_devuelto.hora_inicio = atoi(dest);
  memset(dest, 0, 2);
  subString (cadena, 3, 2, dest);
  valor_devuelto.minuto_inicio = atoi(dest);
  memset(dest, 0, 2);
  subString (cadena, 6, 2, dest);
  valor_devuelto.hora_fin = atoi(dest);
  memset(dest, 0, 2);
  subString (cadena, 9, 2, dest);
  valor_devuelto.minuto_fin = atoi(dest);
  
  memset(dest, 0, 2);
  subString (cadena, 12, 2, dest);
  valor_devuelto.hora2_inicio = atoi(dest);
  memset(dest, 0, 2);
  subString (cadena, 15, 2, dest);
  valor_devuelto.minuto2_inicio = atoi(dest);
  memset(dest, 0, 2);
  subString (cadena, 18, 2, dest);
  valor_devuelto.hora2_fin = atoi(dest);
  memset(dest, 0, 2);
  subString (cadena, 21, 2, dest);
  valor_devuelto.minuto2_fin = atoi(dest);
  return valor_devuelto;
  
}

void pinta_horario(GContext* ctx, char * codigo_horario, int y)
{
        // El 0 equivale a las 10:00, 1 = 10:15, 2 = 10:30, 3 = 10:45, 4 = 11:00, 5 = 11:15, 6 = 11:30
        // 7 = 11:45, 8 = 12:00, 9 = 12:15, 10 = 12:30, 11 = 12:45, 12 = 13:00
    
        // Formula es 4*(hora-10). Por lo tanto, las 12 = 4*(12-10) = 4*2 = 8. La posicion 8 son las 12.
        // Por ello, para pintar hasta las 12, el valor final de x en el for debe ser 12.
 
        char temp_horario1[12];
        char temp_horario2[12];
        
        struct Horario valor_devuelto;
        int valor_inicial, valor_final;
        
        graphics_context_set_fill_color(ctx, GColorBlack );

        if (strcmp(codigo_horario,"mmmm")==0)
        {
            graphics_context_set_fill_color(ctx, GColorWhite );
            graphics_fill_rect(ctx,GRect(25, 24+(y*22), 138, 11),0,GCornerNone );
            graphics_draw_text(ctx, "      LIBRE", fonts_get_system_font(FUENTE), GRect(24, 20+(y*22), 90, 7), GTextOverflowModeFill , GTextAlignmentLeft, NULL);
        }
        else
        {  
          // Primera tanda
          valor_devuelto = devuelve_horario(codigo_horario);
          valor_inicial = (4*(valor_devuelto.hora_inicio-10)) + (valor_devuelto.minuto_inicio/15);
          valor_final = (4*(((valor_devuelto.hora_fin <2) ? 24+valor_devuelto.hora_fin : valor_devuelto.hora_fin) -10)) + (valor_devuelto.minuto_fin/15);
        
          for (int x=valor_inicial;x<valor_final;x++)
            graphics_fill_rect(ctx,GRect(25+(x*2), 14+(22*y), 2, 9),0,GCornerNone );
          if (valor_devuelto.hora_inicio != 99)
            snprintf(temp_horario1, 12, "%02d:%02d-%02d:%02d",valor_devuelto.hora_inicio, valor_devuelto.minuto_inicio, valor_devuelto.hora_fin, valor_devuelto.minuto_fin);
          else
            snprintf(temp_horario1, 12, " ");

        // Segunda tanda
          if (valor_devuelto.hora2_inicio!=99)
            {
            valor_inicial = (4*(valor_devuelto.hora2_inicio-10)) + (valor_devuelto.minuto2_inicio/15);
            valor_final = (4*(((valor_devuelto.hora2_fin <2) ? 24+valor_devuelto.hora2_fin : valor_devuelto.hora2_fin) -10)) + (valor_devuelto.minuto2_fin/15);
        
            for (int x=valor_inicial;x<valor_final;x++)
              graphics_fill_rect(ctx,GRect(25+(x*2), 14+(22*y), 2, 9),0,GCornerNone );
          if (valor_devuelto.hora2_inicio != 99)
            snprintf(temp_horario2, 12, "%02d:%02d-%02d:%02d",valor_devuelto.hora2_inicio, valor_devuelto.minuto2_inicio, valor_devuelto.hora2_fin, valor_devuelto.minuto2_fin);
          else
            snprintf(temp_horario2, 12, " ");
            
            }
          else
            {
            
            snprintf(temp_horario2, 12, " ");

            }
          graphics_context_set_fill_color(ctx, GColorWhite );
          graphics_fill_rect(ctx,GRect(25, 24+(y*22), 138, 11),0,GCornerNone );
          graphics_draw_text(ctx, temp_horario1, fonts_get_system_font(FUENTE), GRect(24, 20+(y*22), 90, 7), GTextOverflowModeFill , GTextAlignmentLeft, NULL);
          graphics_draw_text(ctx, temp_horario2, fonts_get_system_font(FUENTE), GRect(85, 20+(y*22), 100, 7), GTextOverflowModeFill , GTextAlignmentLeft, NULL);
        } 
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
  struct Fecha fecha_actual;
  fecha_actual = devuelve_fecha(ano,num_dia); 
    // left, top, anchura, altura
    // 144x168
    
    // Color del fondo y color del trazo
        
    graphics_context_set_stroke_color(ctx, COLOR_PRINCIPAL);
    graphics_context_set_fill_color(ctx, COLOR_PRINCIPAL);
    graphics_context_set_text_color(ctx, COLOR_PRINCIPAL);  

    // Se pinta los carácteres fijos
    if (chkturnos==0)
    {  
      graphics_draw_text(ctx, "10", fonts_get_system_font(FUENTE), GRect(18, -2, 12, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
      graphics_draw_text(ctx, "15", fonts_get_system_font(FUENTE), GRect(58, -2, 12, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
      graphics_draw_text(ctx, "20", fonts_get_system_font(FUENTE), GRect(99, -2, 12, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
      graphics_draw_text(ctx, "00", fonts_get_system_font(FUENTE), GRect(130, -2, 12, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
    }
    else
    {
      char temp_fecha[30];
      snprintf(temp_fecha, 30, "%s de %d",nombre_mes[fecha_actual.mes],fecha_actual.ano);
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

  int dia_semana = dweek(fecha_actual.ano,fecha_actual.mes,fecha_actual.dia);
  int lunes = num_dia-(dia_semana-1);
  //char * horario_test = "0>Ne1>Lj8>Lj0>Lj5>Lj1>Lj0>Ed0>Lj2>Lj0>Lj0>Lj7>Fj0>Lj0>Lj4>Lj0>Lj0>Hj2>Lj0>Lj0>Lj0>Lj0>Lj2>Mj1>Lj9>Lj0>Lj2>Oj4>Lj0>Lfmmmm0>nn";
char * horario_test = "0DojnnD`nnLd0LojmmmmmmmmnnLl0DojnnD`nnLd0LojmmmmmmmmnnLl0DojnnD`nnLd0LojmmmmmmmmnnLl0DojnnD`nnLd0LojmmmmmmmmnnLl0DojnnD`nnLd";
  char dest[5];
  // Ojo, falla al calcular los dias.
  for (int x=0; x<7; x++)
      {
      int dia_a_pintar;
      if ((lunes +x) < 1)
        dia_a_pintar = devuelve_fecha(ano-1,365 + yisleap(ano) + lunes +x).dia;
      else if ((lunes+x)> 365 + yisleap(ano))
        dia_a_pintar = devuelve_fecha(ano+1,1 +x).dia;
      else
        dia_a_pintar = devuelve_fecha(ano,lunes +x).dia;

      memset(dest, 0, 5);
      /*
      if (dia_a_pintar > numero_de_dias(fecha_actual.mes,fecha_actual.ano)) 
        dia_a_pintar = dia_a_pintar - numero_de_dias(fecha_actual.mes,fecha_actual.ano);
      else if (dia_a_pintar < 1)
        dia_a_pintar = numero_de_dias(fecha_actual.mes-1,fecha_actual.ano) - dia_a_pintar;
      */
    
      // Ojo, aquí hay que añadir el mes y el año al cargar el horario.
      char temp_dia[4];
      subString (horario_test, (dia_a_pintar-1)*4, 4, dest);
      snprintf(temp_dia, 4, "%i",dia_a_pintar);
      graphics_draw_text(ctx, temp_dia, fonts_get_system_font(FUENTE), GRect(0, 20+(x*22), 23, 7), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
      // Ahora, se pintan los horarios de todos los días
      // El string hace referencia al código de horario, y la X, a la posición respecto a la fila
          APP_LOG(APP_LOG_LEVEL_DEBUG, "Para %s", dest);

    pinta_horario(ctx, dest, x);
  }  

  
      /*
      RANGO COMPLETO
      
      for (int x=0; x<15; x++)
      {  
      graphics_fill_rect(ctx,GRect(25+(x*8), 14, 7, 9),0,GCornerNone );
      }
      */
  
  
  // EJEMPLOS
  /*
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
      */
  
  
  // FIN DE EJEMPLO
  
     /* EJEMPLO CON BINARIO (en desuso)
      for (int y=0; y<7; y++)
      { 
        // ********* EJEMPLO CON BINARIOS ********
        char * temp_bin = hex2bin("00FE");
        int inicio1=-1, inicio2=-1, fin1=-1, fin2=-1;
        for (int x=0; x<16; x++)
        { 
        if (temp_bin[x]=='1')  
          {
            if (inicio1<0) inicio1 = x;
            if (fin1>0) 
              if (inicio2<0) inicio2 = x;
            graphics_context_set_fill_color(ctx, GColorBlack );
            graphics_fill_rect(ctx,GRect(25+(x*8), 14+(22*y), 7, 9),0,GCornerNone );
          }
        else
         {
            if (fin1<0) fin1 = x;
            if (inicio2>0) 
              if (fin2<0) fin2 = x;
          }
        }
      if (fin2>13) fin2=fin2-24;
      char temp_horario[12];
      char temp_horario2[12];
      snprintf(temp_horario, 12, "%d:00-%d:00",inicio1+10, fin1+10);
      snprintf(temp_horario2, 12, "%d:00-%d:00",inicio2+10, fin2+10);

      graphics_context_set_fill_color(ctx, GColorWhite );

      graphics_fill_rect(ctx,GRect(25, 24+(y*22), 138, 11),0,GCornerNone );
      graphics_draw_text(ctx, temp_horario, fonts_get_system_font(FUENTE), GRect(24, 20+(y*22), 90, 7), GTextOverflowModeFill , GTextAlignmentLeft, NULL);
      if (fin2>-1)
        graphics_draw_text(ctx, temp_horario2, fonts_get_system_font(FUENTE), GRect(85, 20+(y*22), 100, 7), GTextOverflowModeFill , GTextAlignmentLeft, NULL);

      }  
       FIN DE EJEMPLO CON BINARIO */ 
  
  
      //graphics_fill_rect(ctx,GRect(25, 36, 7, 9),0,GCornerNone );
  
}  // Y termina la función

// Se pulsa el botón arriba
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    num_dia = num_dia-7;
    struct Fecha fecha_actual;
    fecha_actual = devuelve_fecha(ano,num_dia); 
    if (num_dia <1)
    {
      ano--;
      num_dia = 365 + yisleap(ano) + num_dia;
    }
  
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Para %d, la fecha es %d-%d-%d", num_dia, fecha_actual.dia, fecha_actual.mes, fecha_actual.ano);

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
    /*
    char datos[260];
    persist_read_string(PERSIST_KEY_DATOS, datos, PERSIST_STRING_MAX_LENGTH);

    APP_LOG(APP_LOG_LEVEL_DEBUG, "%s",datos);
    */  
  
  
  
  
  
    // Se usa el select para cambiar entre calendario normal y de turnos
}

// Se pulsa el botón abajo
void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    num_dia = num_dia+7;
    if (num_dia > 365 + yisleap(ano))
    {
      ano++;
      num_dia = num_dia - 365 + yisleap(ano);
    }

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
    num_dia_actual = get_yday(mes,dia,ano);
    num_dia = num_dia_actual;
    // Se establece chkturnos a 1 para mostrar el mes y el año. Si es 0, se muestran horas en la zona superior
    chkturnos=1;
    
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
    
    #ifdef PBL_SDK_2
      window_set_fullscreen(window, true);
    #endif
    window_stack_push(window, true /* Animado */);
    window_set_background_color(window, COLOR_FONDO);
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    
    //Se añade la CapaLineas
    CapaLineas = layer_create(bounds);
    layer_set_update_proc(CapaLineas, CapaLineas_update_callback); 
    layer_add_child(window_layer, CapaLineas); 
    
    //persist_write_string(PERSIST_KEY_DATOS,"5A0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0LPA0E0F0");
}




