def __helper():
  dirs = []
  if syn_device[:2] == "10":   dirs.extend(["stub_pll"])
  return dirs

modules = {"local": __helper() }
