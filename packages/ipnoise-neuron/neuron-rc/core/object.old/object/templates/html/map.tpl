<style>
  body * {
    font-family: sans-serif;
    font-size: 14px;
  }
  body.white {
    background-color: white;
    color: black;
  }
  body.black {
    background-color: black;
    color: white;
  }
  span.spaceyspan { margin-right: 20px; }
  div.centeredDiv { text-align: center; }
  li { list-style: none; }
  td { padding-right: 10px; }
</style>

<!--
<script src="static/webgl/lib/pre3d.js"></script>
<script src="static/webgl/lib/pre3d_shape_utils.js"></script>
<script src="static/webgl/lib/demo_utils.js"></script>
<script src="static/webgl/sphere.js"></script>

<div class="centeredDiv">
<canvas id="canvas" width="800" height="600">
  Sorry, this demo requires a web browser which supports HTML5 canvas!
</canvas>
</div>

<p>
JavaScript software 3d renderer
</p>

<table>
<tr><td>Mouse</td><td>&rarr;</td><td>rotate around origin x and y axis</td></tr>
<tr><td>Mouse + ctrl</td><td>&rarr;</td><td>pan x / y</td></tr>
<tr><td>Mouse + shift</td><td>&rarr;</td><td>pan z</td></tr>
<tr><td>Mouse + ctrl + shift</td><td>&rarr;</td><td>adjust focal length</td></tr>
</table>
-->

<div class="object_user_map">
<perl>
    # 1*1 3*3 5*5 7*7 9*9
    my $zoom_level  = 7;
    my $ret         = '<table class="object_user_map_table">';
    my $cur_object  = getCurObject();
    my $neighs      = $cur_object->getNeighs();
    my $rows        = sqrt($zoom_level * $zoom_level);
    my $cols        = $rows;
    my $pos         = 0;
    my $center_row  = int ($rows/2);
    my $center_col  = int ($cols/2);

    my $min_row     = $center_row - 1;
    my $max_row     = $center_row + 1;
    my $min_col     = $center_col - 1;
    my $max_col     = $center_col + 1;

    my $neighs_xy   = {};
    my $x           = $min_col;
    my $y           = $min_row;

    my @tmp_neighs = sort {
        $a->getId() cmp $b->getId()
    } @$neighs;

    use constant RIGHT  => 1;
    use constant DOWN   => 2;
    use constant LEFT   => 3;
    use constant UP     => 4;

    my $state = &RIGHT;

    while (@tmp_neighs){
        my $neigh = shift @tmp_neighs;
        $neighs_xy->{$y}->{$x} = $neigh;

        if (&RIGHT == $state){
            $x++;
            if ($x > $max_col){
                $x = $max_col;
                $state = &DOWN;
            }
        }
        if (&DOWN == $state){
            $y++;
            if ($y > $max_row){
                $y = $max_row;
                $state = &LEFT;
            }
        }
        if (&LEFT == $state){
            $x--;
            if ($x < $min_col){
                $x = $min_col;
                $state = &UP;
            }
        }
        if (&UP == $state){
            $y--;
            if ($y == $min_row){
                $min_row--;
                $min_col--;
                $max_row++;
                $max_col++;
                $y = $min_row;
                $x = $min_col + 1;
                $state = &RIGHT;
            }
        }
    }

    for (my $row = 0; $row < $rows; $row++){
        $ret .= '<tr>';
        for (my $col = 0; $col < $cols; $col++){
            my $td = '';
            if (    $col == $center_col
                &&  $row == $center_row)
            {
                $td .= '<td'
                    .' class="object_user_map_center">'
                    .'<center>';
                $td .= $cur_object->renderEmbedded('icon');
                $td .= '</center></td>';
            } else {
                $td .= '<td'
                    .' class="object_user_map_item">'
                    .'<center>';
                my $neigh = $neighs_xy->{$row}->{$col};
                if ($neigh){
                    $td .= $neigh->renderEmbedded('icon');
                } else {
                    $td .= $cur_object->renderEmbedded(
                       'icon_map_add'
                    );
                }
                $td .= '</center></td>';
            }
            $ret .= $td;
        }
        $ret .= '</tr>';
    }
    $ret .= '</table>';
</perl>
</div>

