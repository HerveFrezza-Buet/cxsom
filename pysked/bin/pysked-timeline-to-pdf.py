import cairo
import sys

if len(sys.argv) < 3:
    print(f'Usage: {sys.argv[0]} <timeline.tml> <output.pdf>')
    sys.exit(0)

data = {}
max_date = 0
for line in open(sys.argv[1], 'r', encoding='utf-8'):
    words = line.split(';')
    tag = words[0]
    date = float(words[1])
    msg = words[2]
    duration = float(words[3])
    max_date = max(max_date, date + duration)
    rgb = [float(c) for c in words[4].split()[0:3]]
    if tag not in data:
        data[tag] = []
    data[tag].append({'date': date, 'msg': msg, 'duration': duration, 'color': rgb})

sorted_keys = sorted(data.keys())
data = {k: data[k] for k in sorted_keys}

unit_per_second = 3

nb_timelines = len(data)
line_width = 3
h_margin = line_width
line_skip = .2
title_skip = 10
min_duration = .2
text_w_offset = .15

width = (max_date+2)*unit_per_second + title_skip
height = nb_timelines*(line_width+line_skip) + 2*h_margin

filename = sys.argv[2]
surface = cairo.PDFSurface (filename, width, height)
ctx = cairo.Context (surface)
ctx.set_source_rgb(1,1,1)
ctx.rectangle(0,0,width,height)
ctx.fill()

ctx.select_font_face('Ubuntu sans',
                     cairo.FONT_SLANT_NORMAL,
                     cairo.FONT_WEIGHT_NORMAL);
ctx.set_font_size(1.);

for i in range(int(max_date + 2)):
    w = title_skip + i*unit_per_second
    if i % 10 == 0:
        ctx.set_line_width(.25)
        ctx.set_source_rgb(.6, .6, .8)
        ctx.move_to(w + text_w_offset, h_margin*.5)
        ctx.show_text(str(i))
        ctx.move_to(w + text_w_offset, height - h_margin*.5)
        ctx.show_text(str(i))
    elif i % 5 == 0:
        ctx.set_line_width(.15)
        ctx.set_source_rgb(.7, .7, 1.)
        ctx.move_to(w + text_w_offset, h_margin*.5)
        ctx.show_text(str(i))
        ctx.move_to(w + text_w_offset, height - h_margin*.5)
        ctx.show_text(str(i))
    else:
        ctx.set_line_width(.1)
        ctx.set_source_rgb(.8, .8, 1.)
    ctx.move_to(w, 0)
    ctx.line_to(w, height)
    ctx.stroke()
    
for idx, (tag, timeline) in enumerate(data.items()):
    base_h = idx*(line_width+line_skip) + h_margin
    ctx.set_source_rgb(0,0,0)
    ctx.move_to(0, base_h+.5*line_width)
    ctx.show_text(tag)
    end = title_skip
    for event in timeline:
        date = event['date']*unit_per_second + title_skip
        duration = event['duration']*unit_per_second
        red, green, blue = event['color']
        ctx.set_source_rgb(red, green, blue)
        start = date
        stop  = date+duration
        start = max(start, end)
        stop  = max(stop, start+min_duration)
        end = stop
                    
        ctx.rectangle(start, base_h, stop - start, line_width)
        ctx.fill()
    
    
ctx.show_page()
print('file "{}" generated.'.format(filename))






