from docutils.parsers.rst.roles import set_classes
from docutils import nodes, utils

def rsource_role(name, rawtext, text, lineno, inliner, options={}, content=[]):
    """Link to a source file stored in a FNAL Redmine repository.

    Returns 2 part tuple containing list of nodes to insert into the
    document and a list of system messages.  Both are allowed to be
    empty.

    :param name: The role name used in the document.
    :param rawtext: The entire markup snippet, with role.
    :param text: The text marked with the role.
    :param lineno: The line number where rawtext appears in the input.
    :param inliner: The inliner instance that called us.
    :param options: Directive options for customization.
    :param content: The directive content for customization.
    """
    app = inliner.document.settings.env.app
    node = make_link_node(rawtext, app, 'source', text, options)
    return [node], []

def rissue_role(name, rawtext, text, lineno, inliner, options={}, content=[]):
    """Link to a Redmine issue.

    Returns 2 part tuple containing list of nodes to insert into the
    document and a list of system messages.  Both are allowed to be
    empty.

    :param name: The role name used in the document.
    :param rawtext: The entire markup snippet, with role.
    :param text: The text marked with the role.
    :param lineno: The line number where rawtext appears in the input.
    :param inliner: The inliner instance that called us.
    :param options: Directive options for customization.
    :param content: The directive content for customization.
    """
    try:
        issue_num = text
        if issue_num <= 0:
            raise ValueError
    except ValueError:
        msg = inliner.reporter.error(
            'Redmine issue number must be a number greater than or equal to 1; '
            '"%s" is invalid.' % text, line=lineno)
        prb = inliner.problematic(rawtext, rawtext, msg)
        return [prb], [msg]
    app = inliner.document.settings.env.app
    node = make_link_node(rawtext, app, 'issue', str(issue_num), options)
    return [node], []

def make_link_node(rawtext, app, type, spec, options):
    """Create a link to a BitBucket resource.

    :param rawtext: Text being replaced with link node.
    :param app: Sphinx application context
    :param type: Link type (sourceCode, issue, etc.)
    :param spec: Specification of the thing to link to
    :param options: Options dictionary passed to role func.
    """
    #
    try:
        base = app.config.fnal_redmine_url
        if not base:
            raise AttributeError
    except AttributeError as err:
        raise ValueError('fnal_redmine_url configuration value is not set (%s)' % str(err))
    #
    try:
        release = app.config.release
        if not base:
            raise AttributeError
    except AttributeError as err:
        raise ValueError('release configuration value is not set (%s)' % str(err))

    slash = '/' if base[-1] != '/' else ''
    ref = base + slash
    prefix = str()
    if type is 'source':
        version = 'v' + release.replace('.', '_')
        #ref += 'projects/cetlib/repository/entry/'
        ref += 'projects/cetlib_except/repository/entry/'
        ref += spec
        ref += '?utf8='
        ref += u'\u2713'
        ref += '&rev='
        ref += version
    elif type is 'issue':
        ref += 'issues/'
        ref += str(spec)
        prefix = '#'
    set_classes(options)
    node = nodes.reference(rawtext, prefix + utils.unescape(spec), refuri=ref,
                           **options)
    return node

def setup(app):
    app.add_role('rsource', rsource_role)
    app.add_role('rissue', rissue_role)
    app.add_config_value('fnal_redmine_url', None, 'env')
