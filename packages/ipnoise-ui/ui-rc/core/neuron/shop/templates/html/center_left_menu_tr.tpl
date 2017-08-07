<style>
        .center_left_menu {
        background-color:   #d5fff2;
        width:              100%;
        height:             100%;
        text-align:         left;
        vertical-align:     top;
        font-family:        Comic Sans MS;
        color:              black;
        font-size:          120%;
        }
        .radius1 {
        background:         white; /* Цвет фона */
        font-family:        Comic Sans MS;
        color:              black;
        border:             1px solid blue; /* Параметры рамки */
        padding:            20px; /* Поля вокруг текста */
        margin-bottom:      5px; /* Отступ снизу */
        }
</style>
<div class="center_left_menu">
    <div><br/>ВСЕ РАЗДЕЛЫ САЙТА:<br/></div> <br/>


<ul>
<perl>
    my $ret     = '';
    my $neighs  = getCurNeighsByType("products");

    foreach my $neigh (@$neighs){
        $ret .= "<li>";
        $ret .= $neigh->renderEmbedded("href");
        $ret .= "</li>";
    }
    return $ret;
</perl>
</ul>

