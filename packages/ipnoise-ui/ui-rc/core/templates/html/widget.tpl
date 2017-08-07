<perl>
    my $ret = '';
    $ret .= processTemplate("header/body.tpl");
    $ret .= processTemplate("navigation/body.tpl");
    return $ret;
</perl>

<table class="center">
<tr>
    <td class="center_left">
        <perl>
            return processCurTemplate(
                "center_left_menu_tr.tpl"
            );
        </perl>
    </td>
    <td class="center_center">
        <perl>
            my $ret         = '';
            my $view        = getContext()->{view};

            my $cur_neuron  = getCurNeuron();
            $ret .= $cur_neuron->renderEmbedded($view);
            return $ret;
        </perl>
    </td>
    <td class="center_right">
        <perl>
            my $ret = processCurTemplate(
                "center_right_menu_tr.tpl"
            );
            return $ret;
        </perl>
    </td>
</tr>

</table>

<perl>
    my $ret = '';
    $ret .= processTemplate("footer/body.tpl");
    return $ret;
</perl>

