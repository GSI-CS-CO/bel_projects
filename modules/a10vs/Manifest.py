def __helper():
  dirs = []
  # a10vs_ip is Quartus 23.1 / Arria 10 IP; Quartus 18.1 cannot analyse it.
  if syn_device[:2] == "10":
    dirs.extend(["src/hdl/a10vs_ip/a10vs_ip/altera_voltage_sensor_231"])
  return dirs

if syn_device[:1] == "5": files = [ "src/hdl/a10vs_pkg.vhd" ]
if syn_device[:4] == "ep2a": files = [ "src/hdl/a10vs_pkg.vhd" ]
if syn_device[:2] == "10": files = [
    "src/hdl/a10vs_pkg.vhd",
    "src/hdl/a10vs_wb.vhd",
    "src/hdl/a10vs.vhd",
    "src/hdl/a10vs_ip/a10vs_ip/synth/a10vs_ip.vhd",
]

modules = {"local": __helper()}
