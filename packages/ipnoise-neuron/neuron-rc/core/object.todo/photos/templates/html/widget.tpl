<div    class="IPNoiseObjectPhotos"
        object_id="<perl>return getCurObjectId()</perl>">

    <perl>
        my $ret    = '';
        my $object = getCurObject();
        my $photos = $object->getNeighsByType("core.object.photo");
        my $photo;

        if (scalar(@$photos)){
            $photo = $photos->[0];
            $ret .= $photo->getId().'<br/>';
            $ret .= $photo->renderEmbedded('widget');

            $ret .= '<br/>';

            $photo = $photos->[0];
            $ret .= $photo->getId().'<br/>';
            $ret .= $photo->renderEmbedded('widget');

            $ret .= '<br/>';

            $photo = $photos->[0];
            $ret .= $photo->getId().'<br/>';
            $ret .= $photo->renderEmbedded('widget');
        }

        return $ret;
    </perl>
</div>

