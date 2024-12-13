import pycxsom as cx

t = cx.typing.Map2D(500, cx.typing.Array(3))

print('type is "{}"'.format(t))
print('  Is it a Type ?   {}'.format(isinstance(t, cx.typing.Type)))
print('  Is it a Scalar ? {}'.format(isinstance(t, cx.typing.Scalar)))
print('  Is it a Pos1D ?  {}'.format(isinstance(t, cx.typing.Pos1D)))
print('  Is it a Pos2D ?  {}'.format(isinstance(t, cx.typing.Pos2D)))
print('  Is it a Array ?  {}'.format(isinstance(t, cx.typing.Array)))
print('  Is it a Map   ?  {}'.format(isinstance(t, cx.typing.Map)))
print('  Is it a Map1D ?  {}'.format(isinstance(t, cx.typing.Map1D)))
print('  Is it a Map2D ?  {}'.format(isinstance(t, cx.typing.Map2D)))
