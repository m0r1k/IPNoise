function Camera(
    a_viewer)
{
    var self = this;

    self.m_viewer = a_viewer;

    // Вектор позиции камеры
    self.m_pos = vec3.create([0, 0, 0]);

    // Куда смотрит камера
    self.m_view = vec3.create([1, 0, 0]);

    // Вектор верхнего направления
    self.m_up = vec3.create([0,  1,  0]);

    // Вектор для стрейфа (движения влево и вправо) камеры
    self.m_strafe = vec3.create([0,  0,  0]);

    // fly mode
    self.m_fly_mode = 0;

    // hide/show picker frame buffer
    self.m_show_pick_fbuffer = 0;

    self.m_logger = new Logger();
    self.m_logger.setPrefix('camera');
}

Camera.prototype.setLogger = function(
    a_logger)
{
    var self = this;
    self.m_logger = a_logger;
}

Camera.prototype.getLogger = function()
{
    var self = this;
    return self.m_logger;
}

Camera.prototype.getGl = function()
{
    var self = this;
    var gl   = self.m_viewer.getGl();
    return gl;
}

Camera.prototype.showPickBuffer = function(
    a_arg)
{
    var self = this;
    if (arguments.length){
        self.m_show_pick_fbuffer = a_arg;
    }
    return self.m_show_pick_fbuffer;
}

Camera.prototype.setFlyMode = function(
    a_arg)
{
    var self = this;
    self.m_fly_mode = a_arg;

    var msg = 'Camera in ';
    msg += a_arg ? 'fly' : 'walk';
    msg += ' mode';
    self.getLogger().pinfo(msg);
}

Camera.prototype.getFlyMode = function()
{
    var self = this;
    return self.m_fly_mode;
}

Camera.prototype.toggleFlyMode = function()
{
    var self = this;
    var cur_val = self.getFlyMode();
    self.setFlyMode(!cur_val);
}

Camera.prototype.cross = function(
    vV1,
    vV2,
    vVector2)
{
    var vNormal     = vec3.create();
    var vVector1    = vec3.create();

    vVector1[X] = vV1[X] - vV2[X];
    vVector1[Y] = vV1[Y] - vV2[Y];
    vVector1[Z] = vV1[Z] - vV2[Z];

    // Если у нас есть 2 вектор (вектор взгляда
    // и вертикальный вектор), у нас есть плоскость,
    // от которой мы можем вычислить угол в 90 градусов.
    // Рассчет cross'a прост, но его сложно запомнить
    // с первого раза. Значение X для вектора
    // = (V1[Y] * V2[Z]) - (V1[Z] * V2[Y])
    vNormal[X] = (
        (vVector1[Y] * vVector2[Z])
            - (vVector1[Z] * vVector2[Y])
    );

    // Значение Y = (V1[Z] * V2[X]) - (V1[X] * V2[Z])
    vNormal[Y] = (
        (vVector1[Z] * vVector2[X])
            - (vVector1[X] * vVector2[Z])
    );

    // Значение Z = (V1[X] * V2[Y]) - (V1[Y] * V2[X])
    vNormal[Z] = (
        (vVector1[X] * vVector2[Y])
            - (vVector1[Y] * vVector2[X])
    );

    // *ВАЖНО* Вы не можете менять этот порядок,
    // иначе ничего не будет работать.
    // Должно быть именно так, как здесь.
    // Просто запомните, если вы ищите Х, вы не используете
    // значение X двух векторов, и то же самое для Y и Z.
    // Заметьте, вы рассчитываете значение из двух других осей,
    //  и никогда из той же самой.

    // Итак, зачем всё это? Нам нужно найти ось,
    // вокруг которой вращаться.
    // Вращение камеры влево и вправо простое
    // - вертикальная ось всегда (0, 1, 0).
    // Вращение камеры вверх и вниз отличается,
    // так как оно происходит вне глобальных осей.
    // Достаньте себе книгу по линейной алгебре, если у вас
    // её ещё нет, она вам пригодится.

    // вернем результат.

    return vNormal;
}

Camera.prototype.magnitude = function(
    vNormal)
{
    // Это даст нам величину нашей нормали, т.е. длину вектора
    // Мы используем эту информацию для нормализации вектора
    // Вот формула: magnitude = sqrt(V[X]^2 + V[Y]^2 + V[Z]^2)
    // где V - вектор.

    var ret;

    ret = Math.sqrt(
            (vNormal[X] * vNormal[X])
        +   (vNormal[Y] * vNormal[Y])
        +   (vNormal[Z] * vNormal[Z])
    );

    return ret;
}

Camera.prototype.normalize = function(
    vVector)
{
    var self = this;

    // Вы спросите, для чего эта ф-я? Мы должны убедиться, что наш вектор нормализирован.
    // Вектор нормализирован - значит, его длинна равна 1. Например,
    // вектор (2,0,0) после нормализации будет (1,0,0).

    // Вычислим величину нормали
    var magnitude = self.magnitude(vVector);

    // Теперь у нас есть величина, и мы можем разделить наш вектор на его величину.
    // Это сделает длинну вектора равной единице, так с ним будет легче работать.
    vVector[X] = vVector[X] / magnitude;
    vVector[Y] = vVector[Y] / magnitude;
    vVector[Z] = vVector[Z] / magnitude;

    return vVector;
}

Camera.prototype.position_camera = function(
    pos_x,
    pos_y,
    pos_z,
    view_x,
    view_y,
    view_z,
    up_x,
    up_y,
    up_z)
{
    var self = this;
    self.m_pos[X]  = pos_x;    // Позиция камеры
    self.m_pos[Y]  = pos_y;    //
    self.m_pos[Z]  = pos_z;    //
    self.m_view[X] = view_x;   // Куда смотрит, т.е. взгляд
    self.m_view[Y] = view_y;   //
    self.m_view[Z] = view_z;   //
    self.m_up[X]   = up_x;     // Вертикальный вектор камеры
    self.m_up[Y]   = up_y;     //
    self.m_up[Z]   = up_z;     //
}

Camera.prototype.headRotate = function(args)
{
    var self    = this;
    var h_speed = args.h_speed;
    var v_speed = args.v_speed;

    if (    h_speed != undefined
        ||  v_speed != undefined)
    {
        var vector = vec3.create();

        vector[X] = self.m_view[X] - self.m_pos[X];
        vector[Y] = self.m_view[Y] - self.m_pos[Y];
        vector[Z] = self.m_view[Z] - self.m_pos[Z];
    }

    if (h_speed != undefined){
        self.m_view[Z] = self.m_pos[Z]
            + Math.sin(h_speed) * vector[X]
            + Math.cos(h_speed) * vector[Z];

        self.m_view[X] = self.m_pos[X]
            + Math.cos(h_speed) * vector[X]
            - Math.sin(h_speed) * vector[Z];
    }

    if (v_speed != undefined){
        self.m_view[Y] += v_speed;
    }
}

Camera.prototype.rotate_view2 = function(
    speed)
{
    var self = this;
}

Camera.prototype.rotate_position = function(
    angle,
    x,
    y,
    z)
{
    var self = this;

    self.m_pos[X] = self.m_pos[X] - self.m_view[X];
    self.m_pos[Y] = self.m_pos[Y] - self.m_view[Y];
    self.m_pos[Z] = self.m_pos[Z] - self.m_view[Z];

    var vVector = self.m_pos;
    var AVector = vec3.create();

    var SinA = Math.sin(Math.PI * angle / 180.0);
    var CosA = Math.cos(Math.PI * angle / 180.0);

    // Найдем новую позицию X для вращаемой точки
    AVector[X] = (CosA + (1 - CosA) * x * x) * vVector[X];
    AVector[X] += ((1 - CosA) * x * y - z * SinA) * vVector[Y];
    AVector[X] += ((1 - CosA) * x * z + y * SinA) * vVector[Z];

    // Найдем позицию Y
    AVector[Y] = ((1 - CosA) * x * y + z * SinA) * vVector[X];
    AVector[Y] += (CosA + (1 - CosA) * y * y) * vVector[Y];
    AVector[Y] += ((1 - CosA) * y * z - x * SinA) * vVector[Z];

    // И позицию Z
    AVector[Z] = ((1 - CosA) * x * z - y * SinA) * vVector[X];
    AVector[Z] += ((1 - CosA) * y * z + x * SinA) * vVector[Y];
    AVector[Z] += (CosA + (1 - CosA) * z * z) * vVector[Z];

    self.m_pos[X] = self.m_view[X] + AVector[X];
    self.m_pos[Y] = self.m_view[Y] + AVector[Y];
    self.m_pos[Z] = self.m_view[Z] + AVector[Z];
}

Camera.prototype.move_camera = function(
    speed) //Задаем скорость
{
    var self    = this;
    var vVector = vec3.create(); //Получаем вектор взгляда

    vVector[X] = self.m_view[X] - self.m_pos[X];
    vVector[Y] = self.m_view[Y] - self.m_pos[Y];
    vVector[Z] = self.m_view[Z] - self.m_pos[Z];

    // Это запрещает камере подниматься вверх
    if (!self.m_fly_mode){
        vVector[Y] = 0;
    }
    vVector = self.normalize(vVector);

    self.m_pos[X]  += vVector[X] * speed;
    self.m_pos[Y]  += vVector[Y] * speed;
    self.m_pos[Z]  += vVector[Z] * speed;

    self.m_view[X] += vVector[X] * speed;
    self.m_view[Y] += vVector[Y] * speed;
    self.m_view[Z] += vVector[Z] * speed;
}

Camera.prototype.strafe = function(
    speed)
{
    var self = this;

    // добавим вектор стрейфа к позиции
    self.m_pos[X] += self.m_strafe[X] * speed;
    self.m_pos[Z] += self.m_strafe[Z] * speed;

    // Добавим теперь к взгляду
    self.m_view[X] += self.m_strafe[X] * speed;
    self.m_view[Z] += self.m_strafe[Z] * speed;
}

Camera.prototype.update = function()
{
    var self    = this;
    var vCross  = self.cross(
        self.m_view,
        self.m_pos,
        self.m_up
    );

    // Нормализуем вектор стрейфа
    self.m_strafe = self.normalize(vCross);
}

Camera.prototype.upDown = function(
    speed)
{
    var self = this;
    self.m_pos[Y] += speed;
}

Camera.prototype.getPosX = function()
{
    // Возвращает позицию камеры по Х
    var self = this;
    var ret  = self.m_pos[X];
    return ret;
}

Camera.prototype.getPosY = function()
{
    // Возвращает позицию камеры по Y
    var self = this;
    var ret  = self.m_pos[Y];
    return ret;
}

Camera.prototype.getPosZ = function()
{
    // Возвращает позицию камеры по Z
    var self = this;
    var ret  = self.m_pos[Z];
    return ret;
}

Camera.prototype.getViewX = function()
{
    // Возвращает позицию взгляда по Х
    var self = this;
    var ret  = self.m_view[X];
    return ret;
}

Camera.prototype.getViewY = function()
{
    // Возвращает позицию взгляда по Y
    var self = this;
    var ret  = self.m_view[Y];
    return ret;
}

Camera.prototype.getViewZ = function()
{
    // Возвращает позицию взгляда по Z
    var self = this;
    var ret  = self.m_view[Z];
    return ret;
}

Camera.prototype.look1 = function(a_tickElapsedTimeMs)
{
    if (a_tickElapsedTimeMs){
        console.log(
              "yaw: '"+yaw+"'"
            +" pitch: '"+pitch+"'"
            +" roll: '"+roll+"'"
        );
    }

    mat4.identity(mvMatrix);

    mat4.translate(mvMatrix, [ 0, 0, -8 ]);
}

Camera.prototype.look2 = function(a_tickElapsedTimeMs)
{
    if (a_tickElapsedTimeMs){
            console.log(
                  "yaw: '"+yaw+"'"
                +" pitch: '"+pitch+"'"
                +" roll: '"+roll+"'"
            );

        if (speed){
            zPos += Math.sin(degToRad(yaw))
//                  * Math.cos(degToRad(yaw))
//                  * Math.cos(degToRad(roll))
                * speed * a_tickElapsedTimeMs;

//            yPos += Math.sin(degToRad(roll))
//                  * Math.cos(degToRad(yaw))
//                  * Math.cos(degToRad(pitch))
//                * speed * a_tickElapsedTimeMs;

//            xPos -= Math.cos(degToRad(yaw))
//                * speed * a_tickElapsedTimeMs;

            // 0.6 "fiddle factor"
            // makes it feel more realistic :-)
            //joggingAngle += elapsed * 0.6;
            //yPos = Math.sin(
            //    degToRad(joggingAngle)
            //) / 20 + 0.4;
        }

        yaw     += yawRate   * a_tickElapsedTimeMs;
        pitch   += pitchRate * a_tickElapsedTimeMs;
        roll    += rollRate  * a_tickElapsedTimeMs;
    }

//            mat4.perspective(
//                45,
//                gl.viewportWidth / gl.viewportHeight,
//                0.1,
//                100.0,
//                pMatrix
//            );
            mat4.identity(mvMatrix);

            mat4.translate(mvMatrix, [ xPos, yPos, zPos ]);

            mat4.rotate(mvMatrix, degToRad(-pitch), [1, 0, 0]);
            mat4.rotate(mvMatrix, degToRad(-yaw),   [0, 1, 0]);
            mat4.rotate(mvMatrix, degToRad(-roll),  [0, 0, 1]);

//            mat4.multiply(mvMatrix, rotationMatrix);

}

Camera.prototype.look3 = function(a_tickElapsedTimeMs)
{
    var self = this;

//    mat4.identity(pMatrix);
//    mat4.translate(pMatrix, [ 0, 0, -12 ]);

/*
    mat4.perspective(
        25,
        gl.viewportWidth / gl.viewportHeight,
        0.1,
        100.0,
        pMatrix
    );
*/

//    mat4.translate(pMatrix, [ 0, 0, -25 ]);

//    mat4.ortho(
//        -1,    1,
//        -1,    1,
//        -1,    1,
//        pMatrix
//    );

//    self.update();
//    self.upDown(self.morik);

    mat4.lookAt(
        self.m_pos,
        self.m_view,
        self.m_up,
        mvMatrix
    );


    if (0){
        var msg = "----------";
        var data = self.m_pos;
        for (var key in data){
            var val = data[key];
            msg += "key: '"+key+"' val: '"+val+"'\n";
        }
        console.log(msg);
    }

//    gl.uniformMatrix4fv(
//        self.pickShader.uPMat,
//        false,
//        pMatrix
//    );


//    gl.uniformMatrix4fv(
//        self.pickShader.uPMat,
//        false,
//        pMatrix
//    );


/*
    mat4.perspective(
        25,
        gl.viewportWidth / gl.viewportHeight,
        0.1,
        100.0,
        pMatrix
    );

    mat4.ortho(
        pMatrix,
        -25,    25,
        -25,    25,
        -25,    25
    );

    gl.uniformMatrix4fv(
        self.pickShader.uPMat,
        false,
        pMatrix
    );


*/
/*
    mat4.lookAt(
        mvMatrix,
        [0,  0, -5],
        [0,  0,  0],
        [0,  1,  0]
    );

    gl.uniformMatrix4fv(
        self.pickShader.uMVMat,
        false,
        mvMatrix
    );
*/
}

Camera.prototype.look = function(a_tickElapsedTimeMs)
{
    var self = this;
    self.look3(a_tickElapsedTimeMs);
}

// ---------------- api ----------------

Camera.prototype.animate = function()
{
}

Camera.prototype.draw = function()
{
    var self = this;
    self.update();
    self.look();
}

Camera.prototype.select   = function(){}
Camera.prototype.deSelect = function(){}

Camera.prototype.handleKeys = function()
{
    var self = this;

    if (currentlyPressedKeys[80]){
        // key 'p'
        self.m_show_pick_fbuffer
            = !self.m_show_pick_fbuffer;
    }

    if (currentlyPressedKeys[70]){
        // key 'f'
        self.toggleFlyMode();
    }
    if (currentlyPressedKeys[67]){
        // key 'c'
        self.togglePointer();
    }

    if (currentlyPressedKeys[37]){
        // left cursor key
        if (currentlyPressedKeys[17]){
            // ctrl pressed
            //rollRate = 0.1;
        } else {
            //yawRate = 0.1;
            self.headRotate({
                h_speed: -0.02
            });
        }
    } else if (currentlyPressedKeys[39]){
        // right cursor key
        if (currentlyPressedKeys[17]){
            // ctrl pressed
            //rollRate = -0.1;
        } else {
            //yawRate = -0.1;
            self.headRotate({
                h_speed: 0.02
            });
        }
    } else {
        if (currentlyPressedKeys[17]){
            // ctrl pressed
            //rollRate = 0;
        } else {
            //yawRate = 0;
        }
    }

    if (currentlyPressedKeys[65]){
        // key A
        //yawRate = 0.1;
        //xPos -= 0.1 * Math.cos(degToRad(yaw));
        //zPos += 0.1 * Math.sin(degToRad(yaw));
        self.strafe(-0.2);
    }
    if (currentlyPressedKeys[68]){
        // key D
        //yawRate = -0.1;
        //xPos += 0.1 * Math.cos(degToRad(yaw));
        //zPos -= 0.1 * Math.sin(degToRad(yaw));
        self.strafe(0.2);
    }

    if (currentlyPressedKeys[38]){
        // up cursor key
        //pitchRate = 0.1;
        self.headRotate({
            v_speed: 0.2
        });
    } else if (currentlyPressedKeys[40]){
        // down cursor key
        //pitchRate = -0.1;
        self.headRotate({
            v_speed: -0.2
        });
    } else {
        //pitchRate = 0;
    }

    if (currentlyPressedKeys[87]){
        // W
        //speed = 0.003;
        self.move_camera(0.5);
    } else if (currentlyPressedKeys[83]){
        // S
        //speed = -0.003;
        self.move_camera(-0.5);
    } else {
        //speed = 0;
    }
}

// -------------------------------------

