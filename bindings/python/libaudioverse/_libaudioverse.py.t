{%import 'macros.t' as macros with context%}
import ctypes
import os.path
import os

#this is a windows hack.
#we want it to find out libsndfile before the system one, so we do this.
libsndfile_module = ctypes.cdll.LoadLibrary(os.path.join(os.path.split(os.path.abspath(__file__))[0], 'libsndfile-1.dll'))
libaudioverse_module = ctypes.cdll.LoadLibrary(os.path.join(os.path.split(os.path.abspath(__file__))[0], 'libaudioverse.dll'))

{%for name, val in constants.iteritems() -%}
{{name}} = {{val}}
{%endfor%}

{%for n, t in typedefs.iteritems()%}
{{n}} = {{t|ctypes_string}}
{%endfor%}

{%for name, info in functions.iteritems()-%}
{{name}} = ctypes.CFUNCTYPE({{info.return_type|ctypes_string}}
{%-if info.args|length > 0%}, {%endif%}{#some functions don't have arguments; if it doesn't, we must avoid the first comma#}
{%-for arg in info.args-%}
{{arg.type|ctypes_string}}
{%-if not loop.last%}, {%endif-%}{#put in a comma and space if needed#}
{%-endfor-%}
)(('{{name}}', libaudioverse_module))
{%endfor%}