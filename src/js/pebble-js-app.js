var appMessageQueue = [];
var maxAppMessageTries = 3;
var appMessageRetryTimeout = 3000;
var appMessageTimeout = 100;

function HTTPGET(url) {
	var req = new XMLHttpRequest();
	req.open("GET", url, false);
	req.send(null);
	return req.responseText;
}

function transforma_horario(horario)
    {
    var valor_retorno="mm";
    if (horario[0]=='L') 
        {
        valor_retorno = "mm";
        }
	else if (horario=="00:00-00:00")
	{
    valor_retorno = "nn";
	}
    else
    {
    var hora1 = horario.substring(0, 5);
    var hora2 = horario.substring(6, 11);
    var todos_los_horarios = ["10:00", "10:15", "10:30", "10:45", "11:00", "11:15", "11:30", "11:45", "12:00", "12:15", "12:30", "12:45", "13:00", "13:15", "13:30", "13:45", "14:00", "14:15", "14:30", "14:45", "15:00", "15:15", "15:30", "15:45", "16:00", "16:15", "16:30", "16:45", "17:00", "17:15", "17:30", "17:45", "18:00", "18:15", "18:30", "18:45", "19:00", "19:15", "19:30", "19:45", "20:00", "20:15", "20:30", "20:45", "ERROR", "21:15", "21:30", "21:45", "22:00", "22:15", "22:30", "22:45", "23:00", "23:15", "23:30", "23:45", "00:00", "00:15", "00:30", "00:45", "01:00","","","21:00"];
     var posicion1 = todos_los_horarios.indexOf(hora1);
     var posicion2 = todos_los_horarios.indexOf(hora2);
         valor_retorno = String.fromCharCode(posicion1+48)+String.fromCharCode(posicion2+48);
    }
     return valor_retorno;
    }


function sendAppMessage() {
  var currentAppMessage;
	if (appMessageQueue.length > 0) {
		currentAppMessage = appMessageQueue[0];
		currentAppMessage.numTries = currentAppMessage.numTries || 0;
		currentAppMessage.transactionId = currentAppMessage.transactionId || -1;
		if (currentAppMessage.numTries < maxAppMessageTries) {
			//console.log('Sending AppMessage to Pebble: ' + JSON.stringify(currentAppMessage.message));
			Pebble.sendAppMessage(
				currentAppMessage.message,
				function(e) {
					appMessageQueue.shift();
					setTimeout(function() {
						sendAppMessage();
					}, appMessageTimeout);
				}, function(e) {
					console.log('Failed sending AppMessage for transactionId:' + e.data.transactionId + '. Error: ' + e.data.error.message);
					appMessageQueue[0].transactionId = e.data.transactionId;
					appMessageQueue[0].numTries++;
					setTimeout(function() {
						sendAppMessage();
					}, appMessageRetryTimeout);
				}
			);
		} else {
			console.log('Failed sending AppMessage for transactionId:' + currentAppMessage.transactionId + '. Bailing. ' + JSON.stringify(currentAppMessage.message));
		}
	}
}


function procesa_csv(url) 
	{
    var datos_dic=[];
		var total_lineas = 0;
        var array_final = "";
        console.log("Voy a procesar "+url);
        var response = HTTPGET(url);
    console.log("Tengo respuesta");
        var lineas = response.split('\n');
        var sumatorio_horario = new Array(10);
        for (var m = 0; m < 13; m++) {
            sumatorio_horario[m] = new Array(31);
            }
		for(var k = 1;k < 33;k++)
			{
			for (var j = 1;j<13;j++)
                {
                sumatorio_horario[j][k]="nnnn";
                }
			}
			

      for(var i = 0;i < lineas.length;)
			{
				
        total_lineas++;
				// Para sacar cada línea del archivo
				var data = [];
				var linea_separada = response.split('\n')[i];
				data[0]=linea_separada.substring(0,linea_separada.indexOf(";"));
				data[1]=linea_separada.substring(linea_separada.indexOf(";")+1,linea_separada.length);
        var str_temp = response.split('\n')[i+1];
				data[2] = str_temp.replace(";", '');
				
				// Para dividir la fecha entre partes y sacar los días de la semana
				var partes = data[1].split("/");
				var fecha = new Date(parseInt(partes[2], 10), parseInt(partes[1], 10) - 1, parseInt(partes[0], 10));
				i = i+2;
				sumatorio_horario[fecha.getMonth()][fecha.getDate()] = transforma_horario(data[0])+transforma_horario(data[2]);
    }
    
    for (var l = 1;l<13;l++)
        {
        array_final = "";
        for(var n = 1;n < 32;n++)
        {
          //console.log("Para "+i+"= "+sumatorio_horario[j][i]);
          array_final = array_final+sumatorio_horario[l][n];
        }
        // AQUI SE AÑADE AL DICCIONARIO QUE SE MANDARÁ AL PEBBLE
        //console.log("Array creada para mes "+l+": "+array_final);
        datos_dic[l-1] = array_final;
        
        appMessageQueue.push({'message': {'mes': l, 'horario': array_final.substring(0,159)}});

      }
      //var dict = {"0" : datos_dic[0], "1": datos_dic[1], "2": datos_dic[2]};
//var dict2 = {"0" : datos_dic[0]};
	/*
  var data0 = datos_dic[0];
	var data1 = datos_dic[1];
	var data2 = datos_dic[2];
	var data3 = datos_dic[3];
	var data4 = datos_dic[4];
	var data5 = datos_dic[5];
	var data6 = datos_dic[6];
	var data7 = datos_dic[7];
	var data8 = datos_dic[8];
	var data9 = datos_dic[9];
	var data10 = datos_dic[10];
	var data11 = datos_dic[11];
  var data12 = datos_dic[0];
  var data13 = datos_dic[0];
  var data14 =datos_dic[0];
  */
  //var dict = {"0" : data0};

    // Vale, ya se el bug. No se pueden enviar más de 124 bytes :(
    // Yo estaba mandando un dic de 124bytes * 14: Por eso petaba
    // pruebo a mandar menos datos
      //var dict = {"0" : "DojnnD`nnLd0Loj0LojmmmmnnD`nnLd0Lojmmmm0LojnnLlnnD`nnLd0Loj0DHjmmmmnnLl0DojnnD`nnLd0LojnnD`mmmmnnD"};

	//Pebble.sendAppMessage(dict);
    
  sendAppMessage();

	}


var ObtenDatos = function() {
  var response = HTTPGET("https://dl.dropboxusercontent.com/u/119376/data.dat");
  //console.log(response);

	var json = JSON.parse(response);
	var data0 = "1nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn";
	var data1 = "2nnnnnnnnnLlnnLlnnD`nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn";
  var data2 = "3DojnnD`nnLd0Loj0LojmmmmnnD`nnLd0Lojmmmm0LojnnLlnnD`nnLd0Loj0DHjmmmmnnLl0DojnnD`nnLd0LojnnD`mmmmnnD`nnLd0LojmmmmnnLlnnLlnnD`";
  var data3 = "4nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn";
  var data4 = "5DojnnD`nnLd0Loj0LojmmmmnnD`nnLd0Lojmmmm0LojnnLlnnD`nnLd0Loj0DHjmmmmnnLl0DojnnD`nnLd0LojnnD`mmmmnnD`nnLd0LojmmmmnnLlnnLlnnD`";
  var data5 = "6DojnnD`nnLd0Loj0LojmmmmnnD`nnLd0Lojmmmm0LojnnLlnnD`nnLd0Loj0DHjmmmmnnLl0DojnnD`nnLd0LojnnD`mmmmnnD`nnLd0LojmmmmnnLlnnLlnnD`";
  var data6 = "7DojnnD`nnLd0Loj0LojmmmmnnD`nnLd0Lojmmmm0LojnnLlnnD`nnLd0Loj0DHjmmmmnnLl0DojnnD`nnLd0LojnnD`mmmmnnD`nnLd0LojmmmmnnLlnnLlnnD`";
  var data7 = "8DojnnD`nnLd0Loj0LojmmmmnnD`nnLd0Lojmmmm0LojnnLlnnD`nnLd0Loj0DHjmmmmnnLl0DojnnD`nnLd0LojnnD`mmmmnnD`nnLd0LojmmmmnnLlnnLlnnD`";
  var data8 = "9DojnnD`nnLd0Loj0LojmmmmnnD`nnLd0Lojmmmm0LojnnLlnnD`nnLd0Loj0DHjmmmmnnLl0DojnnD`nnLd0LojnnD`mmmmnnD`nnLd0LojmmmmnnLlnnLlnnD`";
  var data9 = "1DojnnD`nnLd0Loj0LojmmmmnnD`nnLd0Lojmmmm0LojnnLlnnD`nnLd0Loj0DHjmmmmnnLl0DojnnD`nnLd0LojnnD`mmmmnnD`nnLd0LojmmmmnnLlnnLlnnD`";
  var data10 = "2DojnnD`nnLd0Loj0LojmmmmnnD`nnLd0Lojmmmm0LojnnLlnnD`nnLd0Loj0DHjmmmmnnLl0DojnnD`nnLd0LojnnD`mmmmnnD`nnLd0LojmmmmnnLlnnLlnnD`";
  var data11 = "3DojnnD`nnLd0Loj0LojmmmmnnD`nnLd0Lojmmmm0LojnnLlnnD`nnLd0Loj0DHjmmmmnnLl0DojnnD`nnLd0LojnnD`mmmmnnD`nnLd0LojmmmmnnLlnnLlnnD`";
  var data12 = json.main.data12;
  var data13 = json.main.data13;
  var data14 = json.main.data14;
  var dict = {"0" : data0, "1": data1, "2": data2, "3": data3, "4": data4, "5": data5, "6": data6, "7": data7, "8": data8, "9": data9, "10": data10, "11": data11, "12": data12, "13": data13, "14": data14};
	
	Pebble.sendAppMessage(dict);
};

Pebble.addEventListener("ready",
  function(e) {
	//App is ready to receive JS messages
  }
);

Pebble.addEventListener("appmessage",
  function(e) {
  appMessageQueue = [];

  console.log("Voy a procesar");
    //ObtenDatos();
  procesa_csv("https://dl.dropboxusercontent.com/u/119376/ejemplo.csv");
  }
);