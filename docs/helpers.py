# doing quick testing here for conf.py.in 
# was going to make this a separate file to import modules but 
# the relative path stuff made it harder than easy.
# probably has something to do w/ the fact that cmake is executing
# some template file or something. didn't feel like figuring it out.
# might come back to it.


import os
import sys
sys.path.insert(0, os.path.abspath('_extensions'))


project = 'art'
copyright = '2018'
author = 'artists'

# more general stuff
src = os.environ['MRB_SOURCE']
src_dir = os.environ['MRB_SOURCE']+'/'+project

# html directory
html = os.environ['HTML_DIR']#+'/'project


def get_deps(path2deps):

  products = 0 
  depends_on = []
  
  with open(path2deps) as deps_file:
    for line in deps_file:
      val = line.split()
      if(not val): continue
      if(val[0] == 'end_product_list'): products = 0 
      # product list
      if(products):
        item = [val[0], val[1]]
        depends_on.append(item)
      if(val[0] == 'product'): products = 1

  return depends_on

deps = get_deps(src+'/'+project+'/ups/product_deps')

with open(src+'/'+project+'/docs/depends.rst', 'w+') as deps_page:
#with open('depends.rst', 'w+') as deps_page:
  deps_page.write('|depends| depends')
  deps_page.write('\n=================\n')
  #first = deps[0]
  #deps_page.write(first[0])
  for pkg in deps:
    if(os.path.isdir(html+'/'+pkg[0])):
      print("this is a directory: ", html+'/'+pkg[0])
      deps_page.write('\n(internal link): '+pkg[0])
    else:
      print("this is not a directory: ", html+'/'+pkg[0])
      deps_page.write('\n(external link): '+pkg[0])


print('\n')



















# eof
