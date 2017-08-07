function NeuronParamAddressClass()
{
    var self = AbstractClass('NeuronParamClass');

    var m_MAX_LEVELS    = 100;
    var m_req_id        = 0;
    var m_levels        = [];

    self.setLevels = function(a_levels){
        m_levels = a_levels;
    }
    self.getLevels = function(){
        return m_levels;
    }

    self.loadLevel = function (id, params){
        return self.levelChanged(id, params);
    }

    self.createInput = function(args){
        var type = 'select';
        if (args.type){
            type = args.type;
        }

        if ('select' == type){
            // name
            self.createLevel({
                id:     args.id,
                type:   type
            });
            var select = $(self.getRenderTo()).find('#level'+args.id);
            for (var i = 0; i < args.select.length; i++){
                select.append(
                    $('<option value="'+args.select[i].val+'">'
                        + args.select[i].name
                        + '</option>'
                    )
                );
            }
            // val
            self.createLevel({
                id:     args.id + 1,
                type:   'text_line'
            });
        } else if ('text_area' == type){
            // name
            self.createLevel({
                id:     args.id,
                type:   'label',
                title:  args.title,
                name:   args.name
            });
            // val
            self.createLevel({
                id:     args.id + 1,
                type:   'text_area',
                name:   args.name
            });
        } else {
            self.perror("unsupported level type: '"+type+"'");
            return
        }
    }

    self.save = function (){
        var levels = [];
        for (var i = 0; i < self.getNextLevel(); i += 2){
            var level       = self.getLevel(i);
            var name        = self.getLevelVal(i);
            var type_name   = self.getLevelType(i);
            if (    !name
                ||  !level
                ||  (   'INPUT'  == level.get(0).tagName
                    &&  'submit' == level.attr('type')))
            {
               continue;
            }
            var val         = self.getLevelVal(i + 1);
            var type_val    = self.getLevelType(i + 1);
            levels.push({
                'name':         name,
                'val':          val,
                'type_name':    type_name,
                'type_val':     type_val
            });
        }

        self.updateObject({
            'levels':   levels
        });
    }

    self.addCustomLevels = function(args){
        var id = args.id;

        // Подъезд
        self.createInput({
            id:     id,
            select: [
                {
                    name:   'Подъезд',
                    val:    'podezd'
                }
            ]
        });
        // Домофон
        self.createInput({
            id:     id + 2,
            select: [
                {
                    name:   'Домофон',
                    val:    'domofon'
                }
            ]
        });
        // Квартира
        self.createInput({
            id:     id + 4,
            select: [
                {
                    name:   'Квартира',
                    val:    'kv'
                },
                {
                    name:   'Офис',
                    val:    'office'
                }
            ]
        });
        // Этаж
        self.createInput({
            id:     id + 6,
            select: [
                {
                    name:   'Этаж',
                    val:    'floor'
                }
            ]
        });
        // Заметки
        self.createInput({
            id:     id + 8,
            type:   'text_area',
            title:  'Заметки',
            name:   'notes'
        });
        // сохранить
        self.createLevel({
            id:     id + 10,
            type:   'submit',
            title:  'сохранить',
            click:  function(){
                self.save();
            }
        });
    }

    self.getLevelType = function(id){
        var level = self.getLevel(id);
        if (!level){
            return null;
        }
        var ret      = null;
        var classes  = level.attr('class').split(/\s+/);
        for (var i = 0; i < classes.length; i++){
            var cur_class = classes[i];
            var res = /^type_([^\s]+)/.exec(cur_class);
            if (res && res.length){
                ret = res[1];
            }
        }
        return ret;
    }

    self.getLevelVal = function(id){
        var level = self.getLevel(id);
        if (!level){
            return null;
        }
        var ret      = null;
        var tag_name = level.get(0).tagName;
        if ('SELECT' == tag_name){
            var selected = level.find(
                'option:selected'
            );
            if (selected){
                ret = selected.text();
            }
        } else if ('INPUT' == tag_name){
            ret = level.val();
        } else if ('TEXTAREA' == tag_name){
            ret = level.val();
        } else if ('P' == tag_name){
            ret = level.text();
        } else {
            self.perror("unsupported tag name: '"+tag_name+"' in getLevelVal");
        }

        return ret;
    }

    self.levelChanged  = function(id, params){
        var select = $(self.getRenderTo()).find('#level'+id);
        var packet = {
            'method':   'getLevel'
        };
        var args   = {
            'packet':   packet,
            'success':  function(data){
                if (m_req_id != data.req_id){
                    // skip old answers
                    // chrom send multiple requests while selection :(
                    return;
                }
                var params  = data.params;
                var results = params.results;
                var select  = $(self.getRenderTo()).find('#level'+id);

                // result
                var result;
                var name;
                var val;
                var code;
                var socrname;

                // add empty val
                select.find('option').remove().end().append(
                    '<option value=""></option>'
                );

                var names = [];
                for (var i = 0; i < results.length; i++){
                    result   = results[i];
                    name     = result['name'];
                    code     = result['code'];
                    socrname = result['socrname'];

                    if (socrname){
                        name    = result['socrname'];
                        val     = result['socr'];
                    } else {
                        val     = code;
                    }

                    if (name instanceof Array){
                        for (var k = 0; k < name.length; k++){
                            var cur = name[k];
                            names.push({
                                'name':     cur,
                                'val':      val,
                                'code':     code
                            });
                        }
                    } else {
                        names.push({
                            'name': name,
                            'val':  val,
                            'code': code
                        });
                    }
                }

                if (names.length > 1){
                    names.sort(sortAddr);
                }
                for (var m = 0; m < names.length; m++){
                    var cur = names[m];
                    if (id % 2){
                        select.append(
                            $('<option value="'+cur.name+'">'
                                + cur.name
                                + '</option>'
                            ).attr("code", cur.code)
                        );
                    } else {
                        select.append(
                            $('<option value="'+cur.val+'">'
                                + cur.name
                                + '</option>'
                            ).attr("code", cur.code)
                        );
                    }
                }

                // unbind else select.change will be collected
                select.unbind('change');
                select.focus();
                select.change(function(){
                    var args        = {};
                    var select      = $(self.getRenderTo()).find('#level'+id);
                    var selected    = select.find('option:selected');
                    var val         = selected.val();
                    var code        = selected.attr('code');

                    if (0 == id % 2){
                        args.get_socr = "";
                        if (id >= 2){
                            var prev            = $(self.getRenderTo()).find('#level'+(id - 1));
                            var prev_selected   = prev.find('option:selected');
                            var prev_val        = prev_selected.val();
                            var prev_code       = prev_selected.attr('code');
                            args.code = prev_code;
                            args.code = args.code.replace(/00000000000$/,'%');
                            args.code = args.code.replace(/00000000$/,   '%');
                            args.code = args.code.replace(/00000$/,      '%');
                            args.code = args.code.replace(/00$/,         '%');
                        } else {
                            args.code = '%';
                        }
                        if (val){
                            args.socr = val;
                        }
                    } else {
                        var skip_socr = [];
                        for (var i = id; i >=1; i -= 2){
                            var level = self.getLevel(i);
                            if (!level){
                                break;
                            }
                            skip_socr.push($(self.getRenderTo()).find('#level'+(i-1)).val());
                        }
                        if (skip_socr){
                            args.skip_socr = skip_socr;
                        }
                        if (val){
                            args.code = code;
                            args.code = args.code.replace(/00000000000$/, '%');
                            args.code = args.code.replace(/00000000$/,    '%');
                            args.code = args.code.replace(/00000$/,       '%');
                            args.code = args.code.replace(/00$/,          '%');
                        }
                    }
                    if (args.code.length < 19){
                        var level = self.getLevel(id + 1);
                        if (level){
                            // remove childs
                            self.deleteLevels(id + 1);
                        }
                        if (val){
                            self.createLevel({
                                id:     id + 1,
                                type:   'select'
                            });
                            self.loadLevel(id + 1, args);
                        }
                    }
                });

                if (!names.length){
                    self.deleteLevels(id);
                }
                self.createLevel({
                    id:     (id % 2) ? (id + 1) : (id + 2),
                    type:   'submit',
                    title:  'далее',
                    click:  function(){
                        for (var i = 0; i < self.getNextLevel(); i += 2){
                            var level   = self.getLevel(i);
                            var key     = self.getLevelVal(i);
                            if (    !key
                                ||  !level
                                ||  (   'INPUT'  == level.get(0).tagName
                                    &&  'submit' == level.attr('type')))
                            {
                                self.deleteLevels(i);
                                break;
                            }
                        }
                        self.addCustomLevels({
                            'id': self.getNextLevel()
                        });
                    }
                });
            }
        };

        if (params){
            args.data['params'] = params;
        }

        select.find('option').remove().end().append(
            '<option value="loading">Loading..</option>'
        );

        m_req_id = self.sendApiPacket(args);
    }

    self.getNextLevel = function(){
        var level = -1;
        for (var i = 0; i < m_MAX_LEVELS; i++){
            var res = $(self.getRenderTo()).find("#level"+i);
            if (res.length){
                level = i;
            }
        }
        return level + 1;
    }

    self.getLevel = function(id){
        var level = $(self.getRenderTo()).find('#level'+id);
        if (!level.length){
            level = null;
        }
        return level;
    }

    self.deleteLevels = function(id){
        for (var i = id; i < m_MAX_LEVELS; i++){
            var level = self.getLevel(i);
            if (!level){
                continue;
            }
            if (0 == i % 2){
                var td1 = level.parent();
                var tr  = td1.parent();
                tr.remove();
            } else {
                level.remove();
            }
        }
    }

    self.createLevel = function(args){
        var id          = args.id;
        var type        = args.type;
        var name        = args.name;
        var val         = args.val;
        var read_only   = args.read_only;
        var address     = self.getRenderTo().find('.body').find('table');
        var res;
        var code;

        res = self.getLevel(id);
        if (res){
            // level already exist
            return;
        }

        if (read_only){
            type = 'label';
        }

        if ('select' == type){
            code = $(
                '<select'
                    +' id="level'+id+'"'
                    +' class="type_'+ type +'"'
                    +' name="' + (name ? name : 'level'+id) +'">'
                + (val ? '<option>'+val+'</option>' : '')
                +'</select>'
            );
        } else if ('text_line' == type){
            code = $(
                '<input type="text"'
                    +' id="level'+id+'"'
                    +' class="type_'+ type +'"'
                    +' value="'+val+'"'
                    +' name="' + (name ? name : 'level'+id) +'">'
                +'</input>'
            );
        } else if ('text_area' == type){
            code = $(
                '<textarea'
                    +' id="level'+id+'"'
                    +' class="type_'+ type +'"'
                    +' name="' + (name ? name : 'level'+id) +'">'

                + (val ? val : '')
                +'</textarea>'
            );
        } else if ('label' == type){
            code = $(
                '<p'
                    +' id="level'+id+'"'
                    +' class="type_'+ type +'"'
                    +' name="' + (name ? name : 'level'+id) +'">'
                +args.title
                +'</p>'
            );
        } else if ('submit' == type){
            code = $(
                '<input type="submit"'
                    +' id="level'+id+'"'
                    +' class="type_'+ type +'"'
                    +' name="' + (name ? name : 'level'+id) +'"'
                    +' value="'+args.title+'">'
                +'</input>'
            );
            code.click(args.click);
        } else {
            self.perror("unsupported level type: '"+type+"'");
            return;
        }

        // clear childs
        self.deleteLevels(id + 1);

        if (0 == id % 2){
            var td1     = $('<td/>').append(code);
            var td2     = $('<td/>');
            var tr      = $('<tr/>');

            tr.append(td1);
            tr.append(td2);

            address.append(tr);
        } else {
            var socr = $(self.getRenderTo()).find('#level'+(id - 1));
            var td1  = socr.parent();
            var td2  = td1.next();
            td2.append(code);
        }
    }

    self.render = function(el){
        self.setRenderTo($('#'+self.getId()));

        var level = 0;
        for (var i = 0; i < m_levels.length; i++){
            var name        = m_levels[i].name;
            var val         = m_levels[i].val;
            var type_name   = m_levels[i].type_name;
            var type_val    = m_levels[i].type_val;

            // create new
            self.createLevel({
                id:         level++,
                type:       type_name,
                title:      '<b>'+name+':</b>',
                val:        name,
                read_only:  1
            });
            self.createLevel({
                id:         level++,
                type:       type_val,
                title:      val,
                val:        val,
                read_only:  1
            });

        }
        var level = self.getLevel(0);
        if (!level){
            // create new
            self.createLevel({
                id:     0,
                type:   'select'
            });
            self.loadLevel(0);
        }
    }

    // DO NOT FORGET RETURN SELF
    return self;
}

