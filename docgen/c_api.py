from bindings import get_info, transformers, doc_helper
from bindings import metadata_handler
from . import asciidoc_filters
import jinja2
import yaml

def verify_all_parameters_documented(info, docs):
	for func in info['functions'].itervalues():
		if func.name not in docs:
			continue
		docs[func.name]['params'] = docs[func.name].get('params', dict())
		for p in func.args:
			if p.name not in docs[func.name]['params']:
				if p.name == "destination":
					docs[func.name]['params'][p.name] = "Holds the result of a call to this function."
				elif p.name == "simulationHandle":
					docs[func.name]['params'][p.name] = "The simulation to manipulate."
				elif p.name == "propertyIndex":
					docs[func.name]['params'][p.name] = "The property to manipulate."
				elif p.name == "nodeHandle":
					docs[func.name]['params'][p.name] = "The node to manipulate."
				else:
					raise ValueError("{}: undocumented param {}".format(func.name, p.name))

def make_c_api():
	all_info =get_info.get_all_info()
	functions_by_category=dict()
	for i in all_info['metadata']['function_categories']:
		functions_by_category[i['name']] =[]
	for n, i in all_info['metadata']['functions'].iteritems():
		category=i['category']
		if category not in functions_by_category:
			raise ValueError("{}: {} is not a valid category".format(n, category))
		functions_by_category[category].append(n)
	for i, j in functions_by_category.iteritems():
		j.sort() #alphabetize all of them
		verify_all_parameters_documented(all_info, all_info['metadata']['functions'])
		doc_helper.prepare_docs(all_info,
		param = asciidoc_filters.param, node = asciidoc_filters.node, enum = asciidoc_filters.enum)
	env = jinja2.Environment(loader=jinja2.PackageLoader(__package__, ""), undefined=jinja2.StrictUndefined, trim_blocks=True)
	env.filters.update(transformers.get_jinja2_filters(all_info))
	context=dict()
	context.update(all_info)
	context['functions_by_category'] = functions_by_category
	template=env.get_template("c_api.t")

	return template.render(context)
