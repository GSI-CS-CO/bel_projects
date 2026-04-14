files = [
    "src/hdl/a10vs_pkg.vhd",
    "src/hdl/a10vs_wb.vhd",
    "src/hdl/a10vs.vhd"
]
library = "work"

files = [
    "src/hdl/a10vs_ip/a10vs_ip/synth/a10vs_ip.vhd",
    "src/hdl/a10vs_ip/a10vs_ip/altera_voltage_sensor_231/synth/a10vs_ip_pkg.vhd"
]
library = "a10vs_ip_altera_voltage_sensor_231"

modules = {
    "local" : ["src/hdl/a10vs_ip/a10vs_ip/altera_voltage_sensor_231"]
}
