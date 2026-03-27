package com.furbbs.config;

import org.springframework.context.annotation.Configuration;
import org.springframework.security.saml2.provider.service.registration.InMemoryRelyingPartyRegistrationRepository;
import org.springframework.security.saml2.provider.service.registration.RelyingPartyRegistration;
import org.springframework.security.saml2.provider.service.registration.RelyingPartyRegistrationRepository;

@Configuration
public class SAMLConfig {

    public RelyingPartyRegistrationRepository relyingPartyRegistrationRepository() {
        RelyingPartyRegistration registration = RelyingPartyRegistration.withRegistrationId("saml")
            .entityId("https://furbbs.com/saml/metadata")
            .singleSignOnServiceLocation("https://furbbs.com/api/auth/saml/login")
            .singleLogoutServiceLocation("https://furbbs.com/api/auth/saml/logout")
            .assertionConsumerServiceLocation("https://furbbs.com/api/auth/saml/callback")
            .idpEntityId("https://your-idp.com/entity-id")
            .webSsoUrl("https://your-idp.com/sso")
            .signingX509Certificate("classpath:saml/signing.crt")
            .decryptionX509Certificate("classpath:saml/decryption.crt")
            .build();

        return new InMemoryRelyingPartyRegistrationRepository(registration);
    }
}
