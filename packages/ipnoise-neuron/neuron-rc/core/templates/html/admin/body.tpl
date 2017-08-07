<div id="accordion_bookmarks">
    <h3>Заметки</h3>
    <div>
        <form metod="post" id="updateBookmarks">
        <table style="background-color: #c0c0c0; width: 100%;">
            <tr>
                <th style="width: 100%;">ID</th>
                <th colspan="2">Opers</th>
            </tr>
            <perl>
                my $ret                 = '';
                my $cur_user            = getCurUser();
                my $bookmarks_object    = $cur_user->getBookmarksObject();
                my $bookmarks           = [];
                if ($bookmarks_object){
                    $bookmarks = $bookmarks_object->getNeighs();
                }
                my $i       = 0;
                my $style   = '';
                my $style1  = "background-color: #f0f0f0;";
                my $style2  = "background-color: #e0e0e0;";
                for my $cur_bookmark (@$bookmarks){
                    my $cur_bookmark_id     = $cur_bookmark->getId();
                    my $cur_bookmark_type   = $cur_bookmark->getType();
                    if ($cur_user->getId() eq $cur_bookmark_id){
                        next;
                    }
                    $style = (!$i%2) ? $style1 : $style2;
                    $ret .= '<tr style="'.$style.'"'
                        .' class="'
                            .' neigh'
                            .' object_id_'.$cur_bookmark_id
                            .' object_type_'.$cur_bookmark_type
                            .'"'
                        .'>';
                    $ret .= '<td>';
                    my %args = (
                        object_id   => $cur_bookmark->getId(),
                        title       => $cur_bookmark->getNavName()
                    );
                    $ret .= getHref(%args);
                    $ret .= '</td>';
                    $ret .= '<td>';
                    $ret .= '<img class="neigh_add"'
                        .' src="/static/images/16x16/plus1.png"'
                        .' alt="добавить к текущему нейрону"'
                        .'/>';
                    $ret .= '</td>';
                    $ret .= '<td>';
                    $ret .= '<img class="bookmark_del"'
                        .' src="/static/images/16x16/del1.gif"'
                        .'/>';
                    $ret .= '</td>';

                    $ret .= '</tr>';
                    $i++;
                }
                return $ret;
            </perl>
        </table>
        </form>
        <form metod="post" id="addToBookmarks">
            <input type="submit" value="Добавить текущий нейрон в заметки"></input>
        </form>
    </div>
</div>
<br/>

<div id="accordion_object">
    <perl>
        my $ret = '';
        $ret .= processCurTemplate("admin/body.tpl");
        return $ret;
    </perl>
</div>
<br/>

<div id="accordion_context">
    <h3>Информация о контексте</h3>
    <div>
        <form metod="post" id="updateContext">
            <table>
                <tr>
                    <td>Логин текущего пользователя:</td>
                    <td><input type="text" size="40" style="background-color: #e0e0e0;" readonly
                        value="<perl>return getContextCpp()->getUser()->getParamValue('login');</perl>"></input>
                    </td>
                </tr>
                <tr>
                    <td>ID сессии:</td>
                    <td><input type="text" size="40" style="background-color: #e0e0e0;" readonly
                        value="<perl>return getSession()->getId();</perl>"></input>
                    </td>
                </tr>
            </table>
        </form>
    </div>
</div>
<br/>

<div id="accordion_object_new">
    <h3>Создать соседний нейрон</h3>
    <div>
        <form metod="post" id="createObject">
            <table>
                <tr>
                    <td>Тип нейрона</td>
                    <td>
                        <select id="object_type" name="object_type">
                        <perl>
                            my $ret = '';
                            my $modules_info = getModulesInfo();
                            my @types = sort (keys %$modules_info);
                            foreach my $object_type (@types){
                                $ret .= "<option value=\"$object_type\">$object_type</option>\n";
                            }
                            return $ret;
                        </perl>
                        </select>
                   </td>
               </tr>
               <tr>
                   <td>
                       <input type="submit" value="Создать"></input>
                   </td>
                   <td></td>
               </tr>
           </table>
       </form>
    </div>
</div>

