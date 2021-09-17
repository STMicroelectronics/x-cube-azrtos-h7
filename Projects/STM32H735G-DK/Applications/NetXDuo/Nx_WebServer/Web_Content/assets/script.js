/*------------------------------------------------------------------
* Bootstrap Simple Admin Template
* Version: 2.1
* Author: Alexis Luna
* Website: https://github.com/alexis-luna/bootstrap-simple-admin-template
-------------------------------------------------------------------*/
// Toggle sidebar on Menu button click
$('#sidebarCollapse').on('click', function () {
    $('#sidebar').toggleClass('active');
    $('#body').toggleClass('active');
});
var tx_url = "/GetTXData";
var nx_url = "/GetNXData";
var net_url= "/GetNetInfo";
var tx_cnt_url = "/GetTxCount";
var getpacket_url = "/GetNXPacket";


var dataPoints1 = [];
var xValue1 = 0;
var yValue1 = 0;

var dataPoints3 = [];

var options1 = {
    theme: "dark",
    title: {
        text: "NetXDuo Packet Pool Available",
        fontColor: "Navy"
    },
    data: [{
        type: "spline",
        dataPoints: dataPoints1
    }]
};

var options3 = {
    animationEnabled: true,
    title: {
        text: "Thread's Run Count",                
        fontColor: "Navy"
    },  
    axisY: {
        tickThickness: 0,
        lineThickness: 0,
        valueFormatString: " ",
        includeZero: true,
        gridThickness: 0                    
    },
    axisX: {
        tickThickness: 0,
        lineThickness: 0,
        labelFontSize: 18,
        labelFontColor: "Peru"              
    },
    data: [{
        indexLabelFontSize: 18,
        toolTipContent: "<span style=\"color:#62C9C3\">{indexLabel}:</span> <span style=\"color:#CD853F\"><strong>{y}</strong></span>",
        indexLabelPlacement: "outside",
        indexLabelFontColor: "black",
        indexLabelFontWeight: 500,
        indexLabelFontFamily: "Verdana",
        color: "#0FACAD",
        type: "bar",
        dataPoints: [
            { y : 0, label: "0", indexLabel : "App Main Thread" },
            { y : 0, label: "0", indexLabel : "App Server Thread"},
            { y : 0, label: "0", indexLabel : "LED control Thread"}
        ]
    }]
};


function loadData() {
    jQuery.get(tx_url, function (_data, status) {
        var array = _data.split(',');
        var content =
        document.getElementById("tx_active").innerHTML = "Resumptions : " + array[0];
        document.getElementById("tx_suspended").innerHTML = "Suspentions : " + array[1];
        document.getElementById("idle_returns").innerHTML = "Idle Returns : " + array[2];
        document.getElementById("non_idle_returns").innerHTML = "Non Idle returns : " + array[3];
    });
    jQuery.get(nx_url, function (_data, status) {
        var array = _data.split(',');
        document.getElementById("nx_sent").innerHTML = "Total Bytes Sent  : " + array[0];
        document.getElementById("nx_received").innerHTML = "Total Bytes Received  :  " + array[1];
        document.getElementById("nx_connect").innerHTML = "Total connections  : " + array[2];
        document.getElementById("nx_disconnect").innerHTML = "Total Disconnections  :  " + array[3];
    });
    jQuery.get(net_url, function (_data, status) {
        var array = _data.split(',');
        document.getElementById("net_ip").innerHTML = "IP ADDRESS : " + array[0];
        document.getElementById("net_port").innerHTML = "CONNECTION PORT  :  " + array[1];
    });

$("#packetpoolchart").CanvasJSChart(options1);
$("#threadcountchart").CanvasJSChart(options3);

function addData1(data1) {
    jQuery.get(getpacket_url, function (num1, status) {
            dataPoints1.push({ x: xValue1, y: parseInt(num1)+1});
            xValue1++;
        })   
};

function updateChart() {
    var dps = options3.data[0].dataPoints;
    jQuery.get(tx_cnt_url, function (num3, status) {
        var array1 = num3.split(',');

        dps[0].y = parseInt(array1[1]);
        dps[0].label = array1[1];
        dps[1].y = parseInt(array1[3]);
        dps[1].label = array1[3];
        dps[2].y = parseInt(array1[5]);
        dps[2].label = array1[5];

    options3.data[0].dataPoints = dps;
});    
    $("#threadcountchart").CanvasJSChart().render();
}

addData1();
updateChart();

    var t = setTimeout(function () { loadData() }, 1000);
}

$("#packetpoolchart").CanvasJSChart().render();
//$("#threadcountchart").CanvasJSChart().render();

var led_request = "/LedToggle";
function toggleLed() {
    var led = document.getElementById("greenLed");
    var label = document.getElementById("buttonLabel");
    if(led.checked){
        label.innerHTML = "On";
        jQuery.post("/LedOn");
    }
    else
    {
        label.innerHTML = "Off";
        jQuery.post("/LedOff");
    }
}